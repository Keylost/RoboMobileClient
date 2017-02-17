#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <thread>
#include <mutex>
#include <vector>

#include "queue.hpp"
#include "Engine.hpp"
#include "signs.hpp"

class System
{
	public:
		std::mutex line_mutex; //мутекс для контроля доступа к данным линии
		std::mutex sign_mutex; //мутекс для контроля доступа к данным знаков
		std::mutex engine_mutex; //мутекс для контроля доступа к данным engine
		std::mutex exitState_mutex; //мутекс для контроля доступа к данным engine
		uint32_t  portno;		
		char* host;
		int32_t capture_width;
		int32_t capture_height;
		bool exitState;
		MyRect signarea;
		MyRect linearea;
		
		/*
		 * Очередь полученных с сервера кадров
		 */
		Queue<std::vector<unsigned char>> iqueue;
		
		/*
		 * Глобальная структура с текущими данными линии 
		 */
		line_data Line;
		
		/*
		 * Массив полученнных от сервера знаков
		 */
		std::vector<sign_data> Signs;
		
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
			capture_width = 640;
			capture_height = 360;
			clear_video = false;
		}
		
		void line_get(line_data &destination);
		void line_set(line_data &source);
		void engine_get(Engine &destination);
		void engine_set(Engine &source);
		void signs_get(std::vector<sign_data> &destination);
		void signs_set(std::vector<sign_data> &source);
		bool getExitState();
		void setExitState();
};
