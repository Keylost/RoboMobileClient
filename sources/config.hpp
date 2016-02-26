#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

using namespace std;


class System
{
	public:
		int  portno;
		bool videomaker;
		char videoname[80];
		char* host;
		int capture_width;
		int capture_height;
		
		/*
		 * Если флаг clear_video установлен(true) при записе видео через параметр -v
		 * на кадры не будут нанесены какие-либо метки.
		 * В противном случае будет записано все окно с данными о скорости, направлении и т.д.
		 */
		bool clear_video;
		
		System()
		{
			host = new char[strlen("192.168.111.1")+1];
			memcpy(host, "192.168.111.1", strlen("192.168.111.1")+1);
			portno = 1111;
			videomaker = false;
			capture_width = 640;
			capture_height = 360;
			clear_video = false;
		}
};
