#pragma once
#include "config.hpp"
#include <time.h>

#ifdef __linux__
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#else
#include <windows.h>
typedef int socklen_t;
#pragma comment(lib, "Ws2_32.lib")
#endif

enum dataType
{
	Image_t = 0,
	Engine_t = 1,
	Sing_t = 2,
	Line_t = 3
};

class Client
{
	private:
	System *syst;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int sockfd;
	bool connectState;
	
	public:
	unsigned timeout; //если в течении данного времени не будет связи с сервером, то соединение считается разорванным. В секундах
	time_t timeLastServerSeen;
		
	Client(System &conf);
	bool get_data(void *dst, size_t size);
	void send_data(void *src, size_t size);
	bool connect();
	bool isConnect();
	void disconnect();
};

void client_fnc(System &syst,Client &client);
