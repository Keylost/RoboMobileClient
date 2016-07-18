#include "videomaker.hpp"

using namespace cv;

bool VideoMaker::init(System &syst,int width,int height)
{
	fps = 25;
	size = Size(width, height);
	iscolor = true;
	output.open(syst.videoname, CV_FOURCC('M','J','P','G'), fps, size, iscolor);
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
