#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include "queue.hpp"
#include "Engine.hpp"
#include "signs.hpp"
#include "client.hpp"

using namespace cv;
using namespace std;


class System
{
	public:
		pthread_mutex_t line_mutex; //мутекс для контроля доступа к Engine
		uint32_t  portno;
		bool videomaker;
		char videoname[80];
		char* host;
		int32_t capture_width;
		int32_t capture_height;
		Client client;
		Queue<Mat> iqueue;
		line_data Line;
		vector<sign_data> Signs;
		
		
		/*
		 * Если флаг clear_video установлен(true) при записе видео через параметр -v
		 * на кадры не будут нанесены какие-либо метки.
		 * В противном случае будет записано все окно с данными о скорости, направлении и т.д.
		 */
		bool clear_video;
		
		System()
		{
			line_mutex = PTHREAD_MUTEX_INITIALIZER; //мутекс для контроля доступа к Line
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
};

void System::line_get(line_data &destination)
{
	pthread_mutex_lock(&(line_mutex));
	memcpy(&destination,&Line,sizeof(line_data));
	pthread_mutex_unlock(&(line_mutex));
}

void System::line_set(line_data &source)
{
	pthread_mutex_lock(&(line_mutex));
	memcpy(&Line,&source,sizeof(line_data));
	pthread_mutex_unlock(&(line_mutex));
}
