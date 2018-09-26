#pragma once


#include "config.hpp"

#if defined(RELAYING_MODE)

#ifdef __linux__
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#else
#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <chrono>
#include <thread>


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1211"

struct EngineRelay
{
	uint32_t angle;
	uint32_t speed;
};

void server_fnc(System &syst);

#endif
