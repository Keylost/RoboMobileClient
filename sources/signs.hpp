#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;


enum speeds
{
	speed_crosswalk = 250,
	speed_stop      = 0,
	speed_redlight  = 0
};

enum signs
{
	sign_none          = 0,
	sign_stop          = 1,	
	sign_crosswalk 	   = 2,
	sign_trafficlight  = 3,
};

enum trafficlight_states
{
	redlight    = 0,
	yellowlight = 1,
	greenlight  = 2
};

struct sign_data
{
	Rect area;
	signs sign;
	int state;
};

struct line_data
{
	int32_t robot_center = 323; //point on image
	int32_t center_of_line = 323; //center of black line
	bool on_line = true;
};
