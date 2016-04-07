#include "client.hpp"

Client::Client(System &conf)
{
	syst = &conf;
	//syst.client = this;
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
	//set 4 second timeout
	tv.tv_sec=4; tv.tv_usec=0;
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
	setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char *)&savedtv,savedtv_size);
	setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,(char *)&savedtv,savedtv_size);
	
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

void client_fnc(System &syst,Client &client)
{
	Queue<Mat> &iqueue = syst.iqueue;
	
	dataType tp;
	uint32_t dataSize=0;
	
	vector<uchar> b; //vector for image
	
	while(1)
	{
		client.get_data(&tp, sizeof(uint32_t));
		client.get_data(&dataSize, sizeof(uint32_t));
		
		switch(tp)
		{
			case Image_t:
			{
				b = vector<uchar>(dataSize);
				if(!client.get_data(&b[0], (size_t)dataSize))
				{
					perror("Get data");
					exit(EXIT_FAILURE);
				}
				Object<Mat> *newObj = new Object<Mat>();
				*(newObj->obj) = imdecode(b,1);
				iqueue.push(newObj);
				b.clear();
				break;
			}
			case Line_t:
			{
				line_data locale;
				client.get_data(&locale, (size_t)dataSize);
				syst.line_set(locale);
				break;
			}
			case Sing_t:
			{
				sign_data locale;
				client.get_data(&locale, (size_t)dataSize);
				
				break;
			}
			case Engine_t:
			{
				Engine locale;
				client.get_data(&locale, (size_t)dataSize);
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
		
	}
	return;
}
