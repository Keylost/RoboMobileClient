#pragma once

#include <thread>
#include <mutex>
#include <chrono>

template<typename T>
class Object
{
	public:
	T *obj;
	std::mutex useLock;
	int useCount;
	
	Object();

	~Object();
	
	void free();
	void busy();
};

template<typename T>
class Queue
{
	std::mutex _lock;
	Object<T> *_obj;
	
	public:
	
	Queue();
	
	void push(Object<T> *obj);
	
	Object<T> *waitForNewObject(Object<T> *curObj);
};

/* Это нужно, чтобы избежать проблем с линковкой шаблонных методов */
#include "queue.cppd"
