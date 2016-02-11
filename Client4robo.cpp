#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "client.hpp"
#include <opencv2/opencv.hpp>
#include <ctime>
#include "videomaker.hpp"
#include "telemetry.hpp"
#include "signs.hpp"
#include "config.hpp"
#include "CLP.hpp"
#include <string>

#ifdef _MSC_VER
#define snprintf _snprintf_s 
#endif

using namespace std;
using namespace cv;

Mat crosswalk,stop,green_light,red_light,yellow_light,no_line;
Mat giveway,mainroad;
Mat window;

System syst;
int bordersize =100;
char screenshot_name[16];
VideoMaker video;
Mat img;
Telemetry tel_data;
int power_val = 1;
Client *client;
uint32_t imgsize = 0;
uint16_t width=0; //Resolution
uint16_t height=0;
vector<uchar> b; //vector for image

void error(const char* message); //error function
void show_telemetry(Mat &image,Telemetry &tel_data);
void Power_switcher(int pos, void *ptr);
void init();
void deinit();

int main(int argc, char *argv[])
{
	CLP::parse(argc, argv, syst);
	init();
	//createTrackbar("Engine Power\n1-ON,0-OFF", "Telemetry", &power_val, 1, Power_switcher);
	//printf("Server:  %s:%d \n",syst.host, syst.portno);
	//printf("Press u to take screenshot\n");
		
	while(true)
	{
		if(!client->get_data(&tel_data,sizeof(Telemetry)))
		{
			error("Getting data error");
		}
		client->get_data(&imgsize, 4);
		
		b = vector<uchar>(imgsize);
		client->get_data(&b[0], imgsize);
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
	}

	deinit();
	return 0;
}


















void show_telemetry(Mat &image,Telemetry &tel_data)
{
	//printf("%d %d %d\n",tel_data.speed,tel_data.direction,tel_data.angle);
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
		case sign_mainroad:
			xindent = (panel.cols - mainroad.cols)/2;
			ROI = panel(Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+stop.rows)));
			mainroad.copyTo(ROI);
			break;
		case sign_giveway:
			xindent = (panel.cols - giveway.cols)/2;
			ROI = panel(Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+stop.rows)));
			giveway.copyTo(ROI);
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
	
	//EXPERIMENTAL AREA//
	if(tel_data.mysign.sign==sign_mainroad || tel_data.mysign.sign == sign_giveway)
	{
		int fx = width/2 + tel_data.mysign.area.x;
		int fy = 0 + tel_data.mysign.area.y;
		int ex = fx + tel_data.mysign.area.width;
		int ey = fy + tel_data.mysign.area.height;
		rectangle(image,Point(fx,fy),Point(ex,ey),Scalar(0,255,0), 4, 8);
	}
	//EXPERIMENTAL AREA//
	
	
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

void Power_switcher(int pos, void *ptr)
{
	bool power=true;
	if(pos==1) power = true;
	else power = false;
	client->send_data(&power,1);
	printf("Power changed\n");
	return;
}

void init()
{
	client = new Client(syst);
	
	if(!client->connect())
	{
			
		error("Can't connect to server.");
	}
	
	client->get_data(&width, 2);
	client->get_data(&height, 2);
	
	window = Mat(height,width+bordersize,CV_8UC3,Scalar(255,255,255)); //init main window
	namedWindow("Stream", WINDOW_NORMAL | CV_WINDOW_KEEPRATIO /*| WINDOW_OPENGL*/); //create main window
	
	crosswalk = imread("../img/crosswalk.jpeg",1);
	stop = imread("../img/stop.jpeg",1);
	green_light = imread("../img/green_light_s.jpg",1);
	red_light = imread("../img/red_light_s.jpg",1);
	yellow_light = imread("../img/yellow_light_s.jpg",1);
	no_line = imread("../img/no_line.png",1);
	giveway = imread("../img/ustupi.jpg",1);
	mainroad = imread("../img/glavnaya.jpg",1);
	
	if(!crosswalk.data || !stop.data || !green_light.data || !yellow_light.data || !red_light.data || !no_line.data)
	{
		error("Can't load client data.\n");
	}
	
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
	
	return;
}

void deinit()
{
	video.deinit();
	destroyAllWindows();
	client->disconnect();
}

void error(const char* message)
{
	char buffer[] = "Press any key to exit";
	Mat error_img = Mat(80,500,CV_8UC3,Scalar(255,255,255));
	Scalar color= CV_RGB(0,0,0);
	double size =1.0;
	putText(error_img, string(message), Point(5, 30), FONT_HERSHEY_COMPLEX_SMALL, size, CV_RGB(200,0,0));
	
	putText(error_img, string(buffer), Point(5, 50), FONT_HERSHEY_COMPLEX_SMALL, size, color);
	
	namedWindow("Error",WINDOW_AUTOSIZE); //create error window
	imshow("Error",error_img);
	waitKey(0);
	deinit();
	exit(0);
}
