#include "config.hpp"

void System::line_get(line_data &destination)
{
	line_mutex.lock();
	memcpy(&destination,&Line,sizeof(line_data));
	line_mutex.unlock();
}

void System::line_set(line_data &source)
{
	line_mutex.lock();
	memcpy(&Line,&source,sizeof(line_data));
	line_mutex.unlock();
}

void System::signs_get(vector<sign_data> &destination)
{
	sign_mutex.lock();
	destination = Signs;
	sign_mutex.unlock();
}

void System::signs_set(vector<sign_data> &source)
{
	sign_mutex.lock();
	Signs = source;
	sign_mutex.unlock();
}

void System::engine_get(Engine &destination)
{
	engine_mutex.lock();
	memcpy(&destination,&engine,sizeof(Engine));
	engine_mutex.unlock();
}

void System::engine_set(Engine &source)
{
	engine_mutex.lock();
	memcpy(&engine,&source,sizeof(Engine));
	engine_mutex.unlock();
}

void System::setExitState()
{
	exitState_mutex.lock();
	exitState = true;
	exitState_mutex.unlock();
}

bool System::getExitState()
{
	bool state;
	exitState_mutex.lock();
	state = exitState;
	exitState_mutex.unlock();
	return state;
}

void System::currentJPEG_get(std::vector<uchar> &destination)
{
	destination.clear();
	currentJPEG_mutex.lock();
	destination = std::vector<uchar>(currentJPEG.size());
	memcpy(&destination[0],&currentJPEG[0],currentJPEG.size());
	currentJPEG_mutex.unlock();
}

void System::currentJPEG_set(const std::vector<uchar> &source)
{
	currentJPEG_mutex.lock();
	currentJPEG.clear();
	currentJPEG = std::vector<uchar>(source.size());
	memcpy(&currentJPEG[0],&source[0],source.size());
	currentJPEG_mutex.unlock();
}
