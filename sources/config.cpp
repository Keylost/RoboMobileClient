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
