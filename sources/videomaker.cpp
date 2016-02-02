#include "videomaker.hpp"


bool VideoMaker::init(System &syst)
{
	fps = 25;
	size = Size(640+100, 480);
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
