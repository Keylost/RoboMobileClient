#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "client.hpp"
#include <opencv2/opencv.hpp>
#include <ctime>
#include "videomaker.hpp"
#include "Engine.hpp"
#include "signs.hpp"
#include "config.hpp"
#include "CLP.hpp"
#include <string>
#include <thread>

#ifdef _MSC_VER
#define snprintf _snprintf_s 
#endif

using namespace std;

cv::Mat crosswalk,stop,green_light,red_light,yellow_light,no_line;
cv::Mat start_greenlight, start_redlight;
cv::Mat move_forward, move_left, move_right;
cv::Mat giveway,mainroad;
cv::Mat window;
cv::Mat wroi; //область изображения на window
cv::Mat panel; //область панели информации на window
cv::Mat img_clear;
line_data myline;
vector<sign_data> mysigns;
Engine engine;

System syst;
int bordersize =100;
char screenshot_name[16];
VideoMaker video;

int power_val = 1;
Client *client;
uint32_t imgsize = 0;
uint16_t width=0; //Resolution
uint16_t height=0;

void error(const char* message); //error function
void show_telemetry(cv::Mat &image);
void Power_switcher(int pos, void *ptr);
void init();
void deinit();

int main(int argc, char *argv[])
{
	Object<cv::Mat> *curObj = NULL;
	Queue<cv::Mat> &queue = syst.iqueue;
	
	CLP::parse(argc, argv, syst);
	init();
	/*Создает поток приема данных от сервера*/
	thread thr(client_fnc,ref(syst),ref(*client));
	thr.detach();
	//createTrackbar("Engine Power\n1-ON,0-OFF", "Telemetry", &power_val, 1, Power_switcher);
	printf("Press u to take screenshot\n");
	
	while(true)
	{
		curObj = queue.waitForNewObject(curObj, 5000);
		if(curObj == NULL) break; //завершить цикл по таймауту
		
		cv::Mat &img = *(curObj->obj);
		
		if(syst.clear_video)
		{
			img.copyTo(img_clear);
		}
		
		show_telemetry(img);
		
		if(syst.videomaker)
		{
			if(syst.clear_video) video.write(img_clear);
			else video.write(window);
		}
		cv::imshow("Stream", window);
		
		int c = cv::waitKey(1);
		if(c==27)
		{
			break;
		}
		else if(c==117) //take screenshot u
		{
			snprintf(screenshot_name, sizeof(screenshot_name), "%lu.png", (long unsigned)time(NULL));
			if(imwrite(screenshot_name, window))
			{
				printf("[I]: screenshot %s saved\n", screenshot_name);
			}
			else
			{
				printf("[I]: screenshot cant't be saved. No access/No codecs\n");
			}
		}
		
		curObj->free();
	}

	deinit();
	return 0;
}


















void show_telemetry(cv::Mat &image)
{
	//create window//
	syst.line_get(myline);
	syst.signs_get(mysigns);
	syst.engine_get(engine);
	
	uint8_t *panel_row;
	for(int i=0;i<panel.rows;i++)
	{
		panel_row = (uint8_t*)panel.ptr<uint8_t>(i);
		memset(panel_row,255,panel.cols*3);
	}
	
	cv::rectangle(image, syst.signarea, cv::Scalar(255,0,0), 2, 8);//sign area
	
	cv::Scalar color= CV_RGB(255,0,0);
	double size =1.1;
	char buffer[256];
	snprintf(buffer, sizeof(buffer), "Speed");
	cv::putText(panel, string(buffer), cv::Point(5, height-100), cv::FONT_HERSHEY_COMPLEX_SMALL, size, color);
	snprintf(buffer, sizeof(buffer), "%.2fm/s",engine.real_speed/100.0);
	cv::putText(panel, string(buffer), cv::Point(5, height-80), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.9, color);
	
	//snprintf(buffer, sizeof(buffer), "Power");
	//cv::putText(panel, string(buffer), cv::Point(5, height-30), cv::FONT_HERSHEY_COMPLEX_SMALL, size, color);
	//snprintf(buffer, sizeof(buffer), "%d %%",engine.speed/10);
	//cv::putText(panel, string(buffer), cv::Point(20, height-10), cv::FONT_HERSHEY_COMPLEX_SMALL, size, color);
	
	cv::circle(panel,cv::Point(panel.cols/2,height-60),11,cv::Scalar(0,0,250),1,8);
	if(engine.speed != 0)
	{
		if(engine.direction==1)
		{
			cv::circle(panel,cv::Point(panel.cols/2,height-60),10,cv::Scalar(0,250,0),CV_FILLED,8);
		}
		else
		{
			cv::circle(panel,cv::Point(panel.cols/2,height-60),10,cv::Scalar(0,0,250),CV_FILLED,8);
		}
	}
		
	cv::Mat ROI;
	int xindent;
	int yindent = 0;
	
	for(unsigned i=0;i<mysigns.size();i++)
	{
		yindent += 10;
		//проверить выход за границы области рисования знаков
		if(yindent>=200) break;
		switch(mysigns[i].sign)
		{
			case sign_none:
				break;
			case sign_crosswalk:
				xindent = (panel.cols - crosswalk.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+crosswalk.cols,yindent+crosswalk.rows)));
				yindent += crosswalk.rows;
				crosswalk.copyTo(ROI);			
				break;
			case sign_stop:
				xindent = (panel.cols - stop.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+stop.rows)));
				yindent += stop.rows;
				stop.copyTo(ROI);
				break;
			case sign_mainroad:
				xindent = (panel.cols - mainroad.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+mainroad.rows)));
				yindent += mainroad.rows;
				mainroad.copyTo(ROI);
				break;
			case sign_giveway:
				xindent = (panel.cols - giveway.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+stop.cols,yindent+giveway.rows)));
				yindent += giveway.rows;
				giveway.copyTo(ROI);
				break;
			case sign_trafficlight_red:
				xindent = (panel.cols - green_light.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+green_light.cols,yindent+green_light.rows)));
				yindent += green_light.rows;
				red_light.copyTo(ROI);
				break;
			case sign_trafficlight_yellow:
				xindent = (panel.cols - green_light.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+green_light.cols,yindent+green_light.rows)));
				yindent += green_light.rows;
				yellow_light.copyTo(ROI);
				break;
			case sign_trafficlight_green:
				xindent = (panel.cols - green_light.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+green_light.cols,yindent+green_light.rows)));
				yindent += green_light.rows;
				green_light.copyTo(ROI);
				break;
			case sign_starttrafficlight_red:
				xindent = (panel.cols - start_greenlight.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+start_greenlight.cols,yindent+start_greenlight.rows)));
				yindent += start_greenlight.rows;
				start_redlight.copyTo(ROI);
				break;
			case sign_starttrafficlight_green:
				xindent = (panel.cols - start_greenlight.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+start_greenlight.cols,yindent+start_greenlight.rows)));
				yindent += start_greenlight.rows;
				start_greenlight.copyTo(ROI);
				break;
			case sign_move_forward:
				xindent = (panel.cols - move_forward.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+move_forward.cols,yindent+move_forward.rows)));
				yindent += move_forward.rows;
				move_forward.copyTo(ROI);
				break;
			case sign_move_left:
				xindent = (panel.cols - move_left.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+move_left.cols,yindent+move_left.rows)));
				yindent += move_left.rows;
				move_left.copyTo(ROI);
				break;
			case sign_move_right:
				xindent = (panel.cols - move_right.cols)/2;
				ROI = panel(cv::Rect(cv::Point(xindent,yindent), cv::Point(xindent+move_right.cols,yindent+move_right.rows)));
				yindent += move_right.rows;
				move_right.copyTo(ROI);
				break;
			default:
				break;
		}
		
		int fx = syst.signarea.x + mysigns[i].area.x;
		int fy = syst.signarea.y + mysigns[i].area.y;
		int ex = fx + mysigns[i].area.width;
		int ey = fy + mysigns[i].area.height;
		cv::rectangle(image,cv::Point(fx,fy),cv::Point(ex,ey),cv::Scalar(0,255,0), 4, 8);	
	}
	
	if(myline.on_line)
	{
		cv::rectangle(image,cv::Point(myline.robot_center-5,height-60),cv::Point(myline.robot_center+5,height-1),cv::Scalar(255,255,255), CV_FILLED, 8);
		cv::rectangle(image,cv::Point(myline.robot_center-5,height-60),cv::Point(myline.robot_center+5,height-1),cv::Scalar(255,0,0), 1, 8);
		cv::rectangle(image,cv::Point(myline.center_of_line-5,height-60),cv::Point(myline.center_of_line+5,height-1),cv::Scalar(0,255,0), CV_FILLED, 8);	
	}
	else
	{
		ROI = image(cv::Rect(cv::Point(image.cols/2+no_line.cols/2,image.rows-10), cv::Point(image.cols/2-(no_line.cols-no_line.cols/2),image.rows-no_line.rows-10)));
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
	
	printf("[I]: Connecting to %s:%d...\n",syst.host, syst.portno);
	
	if(!client->connect())
	{
		printf("[E]: Connection failed.\n");
		error("Can't connect to server.");
	}
	printf("Connection was successfully established!\n");	
	
	width = syst.capture_width;
	height = syst.capture_height;
	
	printf("Resolution: %dx%d\n",width,height);
	
	window = cv::Mat(height,width+bordersize,CV_8UC3,cv::Scalar(255,255,255)); //init main window
	wroi = window(cv::Rect(cv::Point(0,0),cv::Point(width,height)));
	panel = window(cv::Rect(cv::Point(width-1,0),cv::Point(width+bordersize-1,height-1)));
	
	cv::namedWindow("Stream", cv::WINDOW_NORMAL | CV_WINDOW_KEEPRATIO /*| WINDOW_OPENGL*/); //create main window
	
	double signHeight = 50;
	cv::Size newSignSize;
	newSignSize.height = (int)signHeight;
	crosswalk = cv::imread("../img/crosswalk.jpeg",1);
	newSignSize.width = (int)(crosswalk.cols/(crosswalk.rows/signHeight));
	cv::resize(crosswalk,crosswalk,newSignSize);
	
	stop = cv::imread("../img/stop.jpeg",1);
	newSignSize.width = (int)(stop.cols/(stop.rows/signHeight));
	cv::resize(stop,stop,newSignSize);
	
	move_forward = cv::imread("../img/move_forward.jpg",1);
	newSignSize.width = (int)(move_forward.cols/(move_forward.rows/signHeight));
	cv::resize(move_forward,move_forward,newSignSize);

	move_left = cv::imread("../img/move_left.jpg",1);
	newSignSize.width = (int)(move_left.cols/(move_left.rows/signHeight));
	cv::resize(move_left,move_left,newSignSize);
	
	move_right = cv::imread("../img/move_right.jpg",1);
	newSignSize.width = (int)(move_right.cols/(move_right.rows/signHeight));
	cv::resize(move_right,move_right,newSignSize);	
	
	green_light = cv::imread("../img/green_light_s.jpg",1);
	newSignSize.width = (int)(green_light.cols/(green_light.rows/signHeight));
	cv::resize(green_light,green_light,newSignSize);
	
	red_light = cv::imread("../img/red_light_s.jpg",1);
	newSignSize.width = (int)(red_light.cols/(red_light.rows/signHeight));
	cv::resize(red_light,red_light,newSignSize);
	
	yellow_light = cv::imread("../img/yellow_light_s.jpg",1);
	newSignSize.width = (int)(yellow_light.cols/(yellow_light.rows/signHeight));
	resize(yellow_light,yellow_light,newSignSize);
	
	start_greenlight = cv::imread("../img/st_green_light_s.jpg",1);
	newSignSize.width = (int)(start_greenlight.cols/(start_greenlight.rows/signHeight));
	cv::resize(start_greenlight,start_greenlight,newSignSize);
	
	start_redlight = cv::imread("../img/st_red_light_s.jpg",1);
	newSignSize.width = (int)(start_redlight.cols/(start_redlight.rows/signHeight));
	cv::resize(start_redlight,start_redlight,newSignSize);	
	
	no_line = cv::imread("../img/no_line.png",1);
	giveway = cv::imread("../img/ustupi.jpg",1);
	newSignSize.width = (int)(giveway.cols/(giveway.rows/signHeight));
	cv::resize(giveway,giveway,newSignSize);
	
	mainroad = cv::imread("../img/glavnaya.jpg",1);
	newSignSize.width = (int)(mainroad.cols/(mainroad.rows/signHeight));
	cv::resize(mainroad,mainroad,newSignSize);	
	
	if(!crosswalk.data || !stop.data || !green_light.data || !yellow_light.data || !red_light.data || !no_line.data || !move_forward.data || !move_left.data || !move_right.data)
	{
		error("Can't load client data.\n");
	}
	
	if(syst.videomaker)
	{
		bool vid_init = false;
		if(syst.clear_video)
		{
			vid_init = video.init(syst,width,height);
		}
		else
		{
			vid_init = video.init(syst,width+bordersize,height);
		}
		if(vid_init)
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
	cv::destroyAllWindows();
	syst.setExitState();
	
	exit(EXIT_SUCCESS);
}

void error(const char* message)
{
	char buffer[] = "Press any key to exit";
	cv::Mat error_img = cv::Mat(80,500,CV_8UC3,cv::Scalar(255,255,255));
	cv::Scalar color= CV_RGB(0,0,0);
	double size =1.0;
	cv::putText(error_img, string(message), cv::Point(5, 30), cv::FONT_HERSHEY_COMPLEX_SMALL, size, cv::Scalar(0,0,200));
	
	cv::putText(error_img, string(buffer), cv::Point(5, 50), cv::FONT_HERSHEY_COMPLEX_SMALL, size, color);
	
	cv::namedWindow("Error",cv::WINDOW_AUTOSIZE); //create error window
	cv::imshow("Error",error_img);
	cv::waitKey(0);
	deinit();
	exit(0);
}
