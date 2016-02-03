#include "client.hpp"

Client::Client(System &conf)
{
	syst = &conf;
}

bool Client::connect()
{
	#if !(defined __linux__)
	WSADATA WsaData;
	int err = WSAStartup(0x0101, &WsaData);
	if (err == SOCKET_ERROR)
	{
		return false;//error("WSAStartup() failed: %ld\n", GetLastError());
		exit(0);
	}
	#endif
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return false;//error("ERROR opening socket");
	server = gethostbyname(syst->host);
	if (server == NULL)
		return false;//error("ERROR, no such host");

	memset((char *) &serv_addr,0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy((char *)&serv_addr.sin_addr.s_addr,
			(char *)server->h_addr,
			server->h_length);
	serv_addr.sin_port = htons(syst->portno);
	
	//set timeouts
	struct timeval tv;
	int tvsz=sizeof(tv);
	struct timeval savedtv;
	socklen_t savedtv_size = sizeof(savedtv);
	//save timeout                                                                            
	getsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&savedtv,&savedtv_size);
	//set 4 second timeout
	tv.tv_sec=4; tv.tv_usec=0;
	if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,tvsz)<0)
	{
		perror("setsockopt ");
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&tv,tvsz)<0)
	{
		perror("setsockopt ");
	}
	
	if (::connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		//reset timeouts                                                                             
		setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&savedtv,savedtv_size);
		setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&savedtv,savedtv_size);
		return false;
	}
	//reset timeouts                                                                             
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&savedtv,savedtv_size);
	setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&savedtv,savedtv_size);
	
	return true;
}

void Client::disconnect()
{
	#ifdef __linux__
	close(sockfd);
	#else
	closesocket(sockfd);
	#endif
}

bool Client::get_data(void *dst, size_t size)
{
	size_t i=0;
	int bytes =0;
	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = recv(sockfd, (char *)dst+i, size-i, 0)) <= 0) 
		{
			return false;//error("getting data error");
		}
	}
	return true;
}

bool Client::send_data(void *src,size_t size)
{
	int bytes = 0;
	size_t i = 0;

	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = send(sockfd, ((char *)src)+i, size-i, 0)) == -1)
		{
			return false;
		}
	}
	return true;
}
