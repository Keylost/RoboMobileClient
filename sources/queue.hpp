#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdint.h>

using namespace std;
//using namespace cv;

template<typename T>
class Object
{
	public:
	T *obj;
	mutex useLock;
	int useCount;
	
	Object();
	
	~Object();
	
	void free();
	void busy();
};

template<typename T>
class Queue
{
	mutex _lock;
	Object<T> *_obj;
	
	public:
	
	Queue();
	
	void push(Object<T> *obj);
	
	Object<T> *waitForNewObject(Object<T> *curObj);
	Object<T> *waitForNewObject(Object<T> *curObj,  uint32_t timeout);
};

/* Это нужно, чтобы избежать проблем с линковкой шаблонных методов */
#include "queue.cppd"
