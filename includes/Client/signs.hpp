#pragma once

enum signs
{
	sign_none          = 0,
	sign_stop          = 1,	
	sign_crosswalk 	   = 2,
	sign_mainroad      = 3,
	sign_giveway       = 4,
	sign_starttrafficlight_green = 5,
	sign_starttrafficlight_red = 6,
	sign_trafficlight_green  = 7,
	sign_trafficlight_yellow = 8,
	sign_trafficlight_red = 9
};

enum trafficlight_states
{
	redlight    = 0,
	yellowlight = 1,
	greenlight  = 2
};

struct MyRect
{
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

struct sign_data
{
	MyRect area;
	signs sign;
	uint32_t detect_time; //время прошедшее с поледне регистрации знака системой
};

struct line_data
{
	int32_t robot_center = 323; //point on image
	int32_t center_of_line = 323; //center of black line
	bool on_line = true;
	bool stop_line = false;
};
