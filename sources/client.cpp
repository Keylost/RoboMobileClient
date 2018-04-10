#include "client.hpp"

using namespace cv;

Client::Client(System &conf)
{
	connectState = false;
	syst = &conf;
	
	timeout = 5;
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
	
	#ifdef __linux__
	int flags = fcntl(sockfd, F_GETFL, 0);
	if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		return false; //Ошибка настройки сокета.
	}
	#else
	unsigned long iMode = 1; //1-включить NONBLOCK режим, 0 - выключить
	if(ioctlsocket(sockfd, FIONBIO, &iMode) != 0)
	{
		return false; //Ошибка настройки сокета.
	}
	#endif
	
	if (::connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0)
	{
		tv.tv_sec = timeout; tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(sockfd, &wfds);
		FD_SET(sockfd, &rfds);
		select(sockfd + 1, &rfds, &wfds, NULL, &tv);
		
		if(FD_ISSET(sockfd, &wfds) || FD_ISSET(sockfd, &rfds))
		{
			socklen_t err_len;
			int error;
			err_len = sizeof(error);
			#ifdef __linux__
			if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &err_len) < 0 || error != 0)
			#else
			if(getsockopt(sockfd, SOL_SOCKET, SO_ACCEPCONN, (char*)&error, &err_len) == SOCKET_ERROR)
			#endif
			{
				return false; //Ошибка соединения.
			}			
		}
		else
		{
			return false;
		}
	}
	
	if(!get_data(&(syst->capture_width), 2)) return false;
	if(!get_data(&(syst->capture_height), 2)) return false;
	if(!get_data(&(syst->signarea),sizeof(Rect))) return false;
	if(!get_data(&(syst->linearea),sizeof(Rect))) return false;
	
	connectState = true;
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
	
	for(i = 0; i < size; i += bytes)
	{
		tv.tv_sec = timeout; tv.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(sockfd, &rfds);
		if(select(sockfd + 1, &rfds, NULL, NULL, &tv) <= 0)
		{
			//todo: проверка на выход по прерыванию
			printf("[E]: Connection with server closed! Bad connection.\n");
			return false;
		}
		else
		{
			bytes = recv(sockfd, (char *)dst+i, size-i, 0);
		}
	}
	
	return true;
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
	Queue<Mat> &iqueue = syst.iqueue;
	
	vector<sign_data> locale;
	sign_data sig;
	
	dataType tp;
	uint32_t dataSize=0;
	
	vector<uchar> b; //vector for image
	
	while(!client.isConnect())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	
	while(1)
	{
		if(syst.getExitState()) break;
		if(!client.get_data(&tp, sizeof(uint32_t))) break;
		if(!client.get_data(&dataSize, sizeof(uint32_t))) break; 
		
		switch(tp)
		{
			case Image_t:
			{
				b = vector<uchar>(dataSize);
				if(!client.get_data(&b[0], (size_t)dataSize)) goto thread_end; //перейти к завершению потока, если не удалось получить данные
				Object<Mat> *newObj = new Object<Mat>();
				*(newObj->obj) = imdecode(b,1);
				iqueue.push(newObj);
				b.clear();
				break;
			}
			case Line_t:
			{
				line_data localeLine;
				if(!client.get_data(&localeLine, (size_t)dataSize)) goto thread_end; //перейти к завершению потока, если не удалось получить данные
				syst.line_set(localeLine);
				break;
			}
			case Sing_t:
			{				
				if(!client.get_data(&sig, (size_t)dataSize)) goto thread_end; //перейти к завершению потока, если не удалось получить данные				
				
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
				if(!client.get_data(&locale, (size_t)dataSize)) goto thread_end; //перейти к завершению потока, если не удалось получить данные
				syst.engine_set(locale);
				break;
			}
			default:
			{
				printf("[W]: Unknown data format\n");
				char *freebuffer = new char[dataSize];
				if(!client.get_data(freebuffer, (size_t)dataSize)) goto thread_end; //перейти к завершению потока, если не удалось получить данные
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
	
	/*
	 * Метка для перехода в конец цикла.
	 * Да, да... Это для goto и за мной уже выехали)
	 */
	thread_end:
	client.disconnect();
	if(syst.getExitState())
	{
		syst.setExitState();
	}
	
	return;
}
