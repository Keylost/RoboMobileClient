#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include "queue.hpp"
#include "Engine.hpp"
#include "signs.hpp"

//using namespace cv;
using namespace std;


class System
{
	public:
		mutex line_mutex; //мутекс для контроля доступа к данным линии
		mutex sign_mutex; //мутекс для контроля доступа к данным знаков
		mutex engine_mutex; //мутекс для контроля доступа к данным engine
		mutex thrState_mutex; //мутекс для контроля доступа к данным thrState
		mutex exitState_mutex; //мутекс для контроля доступа к данным exitState
		
		uint32_t  portno;
		bool videomaker;
		char videoname[80];
		char* host;
		int32_t capture_width;
		int32_t capture_height;
		int thrState; //контроль количества потоков
		bool exitState;
		
		//unsigned int ;
		
		cv::Rect signarea;
		cv::Rect linearea;
		
		/*
		 * Очередь полученных с сервера кадров
		 */
		Queue<cv::Mat> iqueue;
		
		/*
		 * Глобальная структура с текущими данными линии 
		 */
		line_data Line;
		
		/*
		 * Массив полученнных от сервера знаков
		 */
		vector<sign_data> Signs;
		
		/*
		 * 
		 */
		Engine engine;
		
		/*
		 * Если флаг clear_video установлен(true) при записе видео через параметр -v
		 * на кадры не будут нанесены какие-либо метки.
		 * В противном случае будет записано все окно с данными о скорости, направлении и т.д.
		 */
		bool clear_video;
		
		System()
		{
			thrState = 0;
			//timeLastServerSeen = 0;
			
			exitState = false;
			host = new char[strlen("192.168.111.1")+1];
			memcpy(host, "192.168.111.1", strlen("192.168.111.1")+1);
			portno = 1111;
			videomaker = false;
			capture_width = 640;
			capture_height = 360;
			clear_video = false;
		}
		
		void line_get(line_data &destination);
		void line_set(line_data &source);
		void engine_get(Engine &destination);
		void engine_set(Engine &source);
		void signs_get(vector<sign_data> &destination);
		void signs_set(vector<sign_data> &source);
		int  getThrState();
		void setThrState(int a);
		bool getExitState();
		void setExitState();
};
