#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <opencv2/opencv.hpp>

#ifdef __linux__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#else
#ifdef _MSC_VER
#define snprintf _snprintf_s 
#endif
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#endif
#include <ctime>
#include "videomaker.hpp"
#include "telemetry.hpp"
#include "signs.hpp"
#include "config.hpp"
#include "CLP.hpp"

using namespace std;
using namespace cv;


Mat crosswalk,stop,green_light,red_light,yellow_light,no_line;
Mat window;

int sockfd;
int bordersize =100;

bool send_data(void *src,int socket,size_t size)
{
	int bytes = 0;
	size_t i = 0;

	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = send(socket, ((char *)src)+i, size-i, 0)) == -1)
		{
			return false;
		}
	}
	return true;
}

void error(const char* message); //error function

void show_telemetry(Mat &image,Telemetry &tel_data)
{
	//create window//
	Mat wroi = window(Rect(Point(0,0),Point(640,480)));
	Mat panel = window(Rect(Point(640,0),Point(740,480)));
	
	uint8_t *panel_row;
	for(int i=0;i<panel.rows;i++)
	{
		panel_row = (uint8_t*)panel.ptr<uint8_t>(i);
		memset(panel_row,255,panel.cols*3);
	}
	
	rectangle(image,Point(image.cols/2,0), Point(image.cols-1,image.rows/2),Scalar(255,0,0), 2, 8);//sign area
	Rect speed_area(Point(10,image.rows-60),Point(50,image.rows-10));
	//rectangle(image,speed_area,Scalar(0,0,255));//speed area
	
	cv::Scalar color= CV_RGB(255,0,0);
	double size =1.1;
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "Power");
	putText(panel, string(buffer), Point(5, 450), FONT_HERSHEY_COMPLEX_SMALL, size, color);
	snprintf(buffer, sizeof(buffer), "%d %%",tel_data.speed/10);
	putText(panel, string(buffer), Point(20, 470), FONT_HERSHEY_COMPLEX_SMALL, size, color);
	
	circle(panel,Point(panel.cols/2,400),16,Scalar(0,0,250),1,8);
	if(tel_data.speed != 0)
	{
		if(tel_data.direction==1)
		{
			circle(panel,Point(panel.cols/2,400),15,Scalar(0,250,0),CV_FILLED,8);
		}
		else
		{
			circle(panel,Point(panel.cols/2,400),15,Scalar(0,0,250),CV_FILLED,8);
		}
	}
	
	//rectangle(frame,Point(recognize.mysign.area.x+320,recognize.mysign.area.y),Point(recognize.mysign.area.x+320+recognize.mysign.area.width,recognize.mysign.area.y+recognize.mysign.area.height),Scalar(0,255,0), 1, 8);	
	
	Mat ROI;
	int xindent;
	int yindent = 10;
	switch(tel_data.mysign.sign)
	{		
		case sign_none:
			break;
		case sign_crosswalk:
			xindent = (panel.cols - crosswalk.cols)/2;
			ROI = panel(Rect(cv::Point(xindent,yindent), cv::Point(xindent+crosswalk.cols,yindent+crosswalk.rows)));
			crosswalk.copyTo(ROI);			
			break;
		case sign_stop:
			xindent = (panel.cols - stop.cols)/2;
			ROI = panel(Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+stop.rows)));
			stop.copyTo(ROI);
			break;
		case sign_trafficlight:
			xindent = (panel.cols - green_light.cols)/2;
			ROI = panel(Rect(cv::Point(xindent,yindent), cv::Point(xindent+green_light.cols,yindent+green_light.rows)));
			
			if(tel_data.mysign.state==greenlight)
			{
				green_light.copyTo(ROI);				
			}
			else if(tel_data.mysign.state==redlight)
			{
				red_light.copyTo(ROI);
			}
			else if(tel_data.mysign.state==yellowlight)
			{
				yellow_light.copyTo(ROI);
			}
			else printf("Sign state data corrupted\n");
			break;
		default:
			break;
	}
	
	
	
	if(tel_data.myline.on_line)
	{
		rectangle(image,Point(tel_data.myline.robot_center-5,420),Point(tel_data.myline.robot_center+5,480),Scalar(255,255,255), CV_FILLED, 8);
		rectangle(image,Point(tel_data.myline.robot_center-5,420),Point(tel_data.myline.robot_center+5,480),Scalar(255,0,0), 1, 8);
		rectangle(image,Point(tel_data.myline.center_of_line-5,420),Point(tel_data.myline.center_of_line+5,480),Scalar(0,255,0), CV_FILLED, 8);	
	}
	else
	{
		ROI = image(Rect(cv::Point(image.cols/2+no_line.cols/2,image.rows-10), cv::Point(image.cols/2-(no_line.cols-no_line.cols/2),image.rows-no_line.rows-10)));
		no_line.copyTo(ROI);	
	}
	
	image.copyTo(wroi);	
}

void get_data(void *dst, int socket, size_t size)
{
	size_t i=0;
	int bytes =0;
	for (i = 0; i < size; i += bytes) {
		if ((bytes = recv(socket, (char *)dst+i, size-i, 0)) <= 0) {
			error("getting data error");
		}
	}
}

void Power_switcher(int pos, void *ptr)
{
	bool power=true;
	if(pos==1) power = true;
	else power = false;
	send_data(&power,sockfd,1);
	printf("Power changed\n");
	return;
}

int main(int argc, char *argv[])
{
#if !(defined __linux__)
	WSADATA WsaData;
	int err = WSAStartup(0x0101, &WsaData);
	if (err == SOCKET_ERROR)
	{
		printf("WSAStartup() failed: %ld\n", GetLastError());
		exit(0);
	}
#endif

	uint32_t imgsize = 0;
	char screenshot_name[16];
	namedWindow("Stream", WINDOW_NORMAL | CV_WINDOW_KEEPRATIO /*| WINDOW_OPENGL*/); //window for image
	VideoMaker video;
	Mat img;
	Telemetry tel_data;
	int power_val = 1;
	createTrackbar("Engine Power\n1-ON,0-OFF", "Telemetry", &power_val, 1, Power_switcher);
	struct sockaddr_in serv_addr;
	struct hostent *server;
	std::vector<uchar> b; //vector for image
	uint16_t width=0;
	uint16_t height=0;
	System syst;
	CLP::parse(argc, argv, syst);

	if(syst.videomaker)
	{
		if(video.init(syst))
		{
			printf("VideoMaker started. File: %s \n",syst.videoname);			
		}
		else
		{
			printf("VideoMaker error. No access/No codecs \n");			
		}
	}
	
	crosswalk = imread("../img/crosswalk.jpeg",1);
	stop = imread("../img/stop.jpeg",1);
	green_light = imread("../img/green_light_s.jpg",1);
	red_light = imread("../img/red_light_s.jpg",1);
	yellow_light = imread("../img/yellow_light_s.jpg",1);
	no_line = imread("../img/no_line.png",1);
	
	if(!crosswalk.data || !stop.data || !green_light.data || !yellow_light.data || !red_light.data || !no_line.data)
	{
		printf("Can't load client data.\n");
		exit(10);
	}

	printf("Server:  %s:%d \n",syst.host, syst.portno);
	printf("Client started. Press ESC to exit\n");
	printf("Press u to take screenshot\n");

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(syst.host);
	if (server == NULL)
		error("ERROR, no such host");

	memset((char *) &serv_addr,0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr,
			(char *)server->h_addr,
			server->h_length);
	serv_addr.sin_port = htons(syst.portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR connecting");
	}

	get_data(&width, sockfd, 2);
	get_data(&height, sockfd, 2);
	printf("Resolution: %ix%i\n", (int)width, (int)height);
	window = Mat(height,width+bordersize,CV_8UC3,Scalar(255,255,255));
	while(true)
	{
		get_data(&tel_data,sockfd,sizeof(Telemetry));		

		get_data(&imgsize, sockfd, 4);
		b = vector< uchar >(imgsize);
		get_data(&b[0], sockfd, imgsize);
		img = imdecode(b,1);
		show_telemetry(img,tel_data);

		if(syst.videomaker) video.write(window);
		imshow("Stream", window);
		int c = waitKey(1);
		if(c==27) break;
		else if(c==117) //take screenshot u
		{
			snprintf(screenshot_name, sizeof(screenshot_name), "%lu.png", time(NULL));
			if(imwrite(screenshot_name, img))
			{
				printf("[I]: screenshot %s saved\n", screenshot_name);
			}
			else
			{
				printf("[I]: screenshot cant't be saved. No access/No codecs\n");
			}
		}
		b.clear();
		//img.release();	
	}

	video.deinit();
	destroyAllWindows();

#ifdef __linux__
	close(sockfd);
#else
	closesocket(sockfd);
#endif
	return 0;
}

void error(const char* message)
{
	printf("%s\n", message);
	exit(1);
}
