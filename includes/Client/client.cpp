#include "client.hpp"
#include <iostream>

/* поддерживать соединение с другой стороной */
void keepAliveEnable(int sockfd)
{
	int32_t optval = 1;
	size_t optlen = sizeof(optval);
	if(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, optlen) < 0)
	{
		perror("keepAliveEnable()");
#ifdef __linux__
		close(sockfd);
#else
		closesocket(sockfd);
#endif
		exit(EXIT_FAILURE);
	}
}

Client::Client(System &conf)
{
	connectState = false;
	syst = &conf;
}

bool Client::isConnect()
{
	return connectState;
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
	getsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&savedtv,&savedtv_size);
	//set 1 second timeout
	tv.tv_sec=7; tv.tv_usec=0;
	if (setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,tvsz)<0)
	{
		perror("setsockopt ");
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,tvsz)<0)
	{
		perror("setsockopt ");
	}
	
	if (::connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		//reset timeouts                                                                             
		setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&savedtv,savedtv_size);
		setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&savedtv,savedtv_size);
		return false;
	}
	//reset timeouts                                                                             
	//setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&savedtv,savedtv_size);
	//setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&savedtv,savedtv_size);
	
	keepAliveEnable(sockfd);
	
	get_data(&(syst->capture_width), 2);
	get_data(&(syst->capture_height), 2);
	get_data(&(syst->signarea),sizeof(MyRect));
	get_data(&(syst->linearea),sizeof(MyRect));
	
	connectState = true;
	return connectState;
}

void Client::disconnect()
{
	#ifdef __linux__
	close(sockfd);
	#else
	closesocket(sockfd);
	#endif
}

void Client::get_data(void *dst, size_t size)
{
	
	
	size_t i=0;
	int bytes =0;
	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = recv(sockfd, (char *)dst+i, size-i, 0)) <= 0) 
		{
			printf("client_fnc(): Getting data error. Server is dead\n");
			disconnect();
			exit(EXIT_FAILURE);
		}
	}
	return;
}

void Client::send_data(void *src,size_t size)
{
	int bytes = 0;
	size_t i = 0;

	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = send(sockfd, ((char *)src)+i, size-i, 0)) <= 0)
		{
			printf("client_fnc(): Sending data error. Server is dead\n");
			disconnect();
			exit(EXIT_FAILURE);
		}
	}
	return;
}

void client_fnc(System &syst,Client &client)
{
	
	Queue<std::vector<unsigned char>> &iqueue = syst.iqueue;
	
	std::vector<sign_data> locale;
	sign_data sig;
	
	dataType tp;
	uint32_t dataSize=0;
	
	std::vector<unsigned char> b; //vector for image
	
	while(!client.isConnect())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	
	while(1)
	{
		if(syst.getExitState()) break;
		client.get_data(&tp, sizeof(uint32_t));
		client.get_data(&dataSize, sizeof(uint32_t));
		
		switch(tp)
		{
			case Image_t:
			{
				b.resize(dataSize);				
				client.get_data(&b[0], (size_t)dataSize);
				Object<std::vector<unsigned char>> *myObj = new Object<std::vector<unsigned char>>();				
				*(myObj->obj) = b;
				iqueue.push(myObj);
				break;
			}
			case Line_t:
			{
				line_data localeLine;
				client.get_data(&localeLine, (size_t)dataSize);
				syst.line_set(localeLine);
				break;
			}
			case Sing_t:
			{				
				client.get_data(&sig, (size_t)dataSize);				
				
				unsigned i=0;
				for(;i<locale.size();i++)
				{
					if(locale[i].sign == sig.sign)
					{
						locale[i] = sig;
						break;
					}
				}
				if(i==locale.size())
				{
					locale.push_back(sig);
				}				
				
				break;
			}
			case Engine_t:
			{
				Engine locale;
				client.get_data(&locale, (size_t)dataSize);
				syst.engine_set(locale);
				break;
			}
			default:
			{
				printf("[W]: Unknown data format\n");
				char *freebuffer = new char[dataSize];
				client.get_data(freebuffer, (size_t)dataSize);
				delete[] freebuffer;
				break;
			}
		}
		
		for(unsigned i=0;i<locale.size();i++)
		{
			locale[i].detect_time+=50;
			if(locale[i].detect_time>1000)
			{
				locale.erase(locale.begin()+i);
			}
		}
		syst.signs_set(locale);
		
	}
	
	client.disconnect();
	
	return;
}
