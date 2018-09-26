#include "videomaker.hpp"

using namespace cv;

bool VideoMaker::init(const char* fileName,int width,int height, int codec)
{
	fps = 25;
	size = Size(width, height);
	iscolor = true;
	//output.open(syst.videoname, CV_FOURCC('M','J','P','G'), fps, size, iscolor);
	output.open(fileName, codec, fps, size, iscolor);
    if(!output.isOpened())
    {
		return false;
	}
	
	return true;
}

void VideoMaker::write(Mat& frame)
{
	output.write(frame);
}

void VideoMaker::deinit()
{
	output.release();
}
