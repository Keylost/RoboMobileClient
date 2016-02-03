#pragma once
#include "config.hpp"
#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#else
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

class Client
{
	private:
	System *syst;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int sockfd;
	
	public:
	Client(System &conf);
	bool get_data(void *dst, size_t size);
	bool send_data(void *src, size_t size);
	bool connect();
	void disconnect();
};
