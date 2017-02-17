#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "config.hpp"
#include <string>

using namespace std;

class CLP
{
	public:
	static void usage(const char *progname);
	static void parse(int argc, char **argv, System &syst);
};
