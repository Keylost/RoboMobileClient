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
