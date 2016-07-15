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

using namespace cv;
using namespace std;


class System
{
	public:
		mutex line_mutex; //мутекс для контроля доступа к данным линии
		mutex sign_mutex; //мутекс для контроля доступа к данным знаков
		mutex engine_mutex; //мутекс для контроля доступа к данным engine
		mutex exitState_mutex; //мутекс для контроля доступа к данным engine
		uint32_t  portno;
		bool videomaker;
		char videoname[80];
		char* host;
		int32_t capture_width;
		int32_t capture_height;
		bool exitState;
		Rect signarea;
		Rect linearea;
		
		/*
		 * Очередь полученных с сервера кадров
		 */
		Queue<Mat> iqueue;
		
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
		bool getExitState();
		void setExitState();
};
