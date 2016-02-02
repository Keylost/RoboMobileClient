#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>

using namespace std;


class System
{
	public:
		int  portno;
		bool videomaker;
		char videoname[80];
		char* host;
		System()
		{
			host = new char[strlen("192.168.111.1")+1];
			memcpy(host, "192.168.111.1", strlen("192.168.111.1")+1);
			portno = 1111;
			videomaker = false;
		}
};
