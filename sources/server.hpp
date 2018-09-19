#pragma once

#ifdef WIN32
#define RELAYING_MODE
#endif

#if defined(RELAYING_MODE)
#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "config.hpp"

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <chrono>
#include <thread>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1211"

struct EngineRelay
{
	uint32_t angle;
	uint32_t speed;
};

void server_fnc(System &syst);

#endif
