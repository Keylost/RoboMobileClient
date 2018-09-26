#pragma once
#include <stdint.h>
#include <opencv2/opencv.hpp>
#include "config.hpp"

//using namespace cv;
using namespace std;

class VideoMaker
{
private:
	double fps;
	cv::Size size;	
	cv::VideoWriter output;
	bool iscolor;
public:	
	bool init(const char* fileName,int width,int height, int codec);
	void write(cv::Mat& frame);
	void deinit();
};
