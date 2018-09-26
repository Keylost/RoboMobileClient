#include "server.hpp"

#if defined(RELAYING_MODE)

#if defined(WIN32)

void server_fnc(System &syst)
{
	Engine localEngine;
	std::vector<uchar> lastjpeg;
	uint32_t lastjpegSize = 0;
	while (true)
	{
		EngineRelay strToSnd;
		strToSnd.angle = 90; //at center by default 
		strToSnd.speed = 20;

		WSADATA wsaData;
		int iResult;

		SOCKET ListenSocket = INVALID_SOCKET;
		SOCKET ClientSocket = INVALID_SOCKET;

		struct addrinfo *result = NULL;
		struct addrinfo hints;

		int iSendResult;
		char recvbuf[DEFAULT_BUFLEN];
		int recvbuflen = DEFAULT_BUFLEN;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			printf("WSAStartup failed with error: %d\n", iResult);
			return 1;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AlastjpegSizeF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		if (iResult != 0) {
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return 1;
		}
socklen_t clilen;
		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR) {
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		// No longer need server socket
		closesocket(ListenSocket);

		int simpleTimer = 0;
		int simDir = 1;
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			if(syst.getExitState())
			{
				closesocket(ClientSocket);
				WSACleanup();
				while(true)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}
			syst.engisocklen_t clilen;ne_get(localEngine);
			syst.currentJPEG_get(lastjpeg);
			strToSnd.angle = localEngine.angle;
			strToSnd.speed = localEngine.real_speed;
			lastjpegSize = (uint32_t)lastjpeg.size();

			iSendResult = send(ClientSocket, (char *)(&strToSnd), sizeof(EngineRelay), 0);
			if (iSendResult == SOCKET_ERROR) goto sockerr;
			iSendResult = send(ClientSocket, (char *)(&lastjpegSize), sizeof(uint32_t), 0);
			if (iSendResult == SOCKET_ERROR) goto sockerr;
			iSendResult = send(ClientSocket, (char *)(&lastjpeg[0]), lastjpegSize, 0);
			
			sockerr:
			if (iSendResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				printf("Restarting server...\n");
				closesocket(ClientSocket);
				WSACleanup();
				break;
			}
		}
				//printf("Bytes sent: %d\n", iSendResult);
	}
	return;
}

#else

void sockOptEnable(int sockfd, int optName)
{
	int32_t optval = 1;
	size_t optlen = sizeof(optval);
	if(setsockopt(sockfd, SOL_SOCKET, optName, (char *)&optval, optlen) < 0)
	{
		perror("sockOptEnable()");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
}

bool send_data(void *src,int socket,size_t size)
{
	int bytes = 0;
	size_t i = 0;
	for (i = 0; i < size; i += bytes) 
	{
		if ((bytes = send(socket, ((uint8_t *)src)+i, size-i, MSG_NOSIGNAL)) <= 0) 
		{
			printf("Sending data error");
			return false;
		}
	}
	return true;
}

void server_fnc(System &syst)
{
	int portno = 1211;
	Engine localEngine;
	std::vector<uchar> lastjpeg;
	uint32_t lastjpegSize = 0;
	while(true)
	{
		EngineRelay strToSnd;
		strToSnd.angle = 90; //at center by default 
		strToSnd.speed = 20;
		printf("Relaying server started\n");
		socklen_t clilen;
		int sockfd = 0;
		int newsockfd = 0;
		struct sockaddr_in serv_addr, cli_addr;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
		{
			printf("ERROR opening socket\n");
		}
		sockOptEnable(sockfd,SO_KEEPALIVE);
		sockOptEnable(sockfd,SO_REUSEADDR);

		bzero((char *) &serv_addr, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = INADDR_ANY;
		serv_addr.sin_port = htons(portno);
			
		while(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0)
		{
			printf("ERROR binding socket\n");
			sleep(1); //wait 1 sec befor next try
		}

		listen(sockfd,5);
		clilen = sizeof(cli_addr);
		printf("waiting for client\n");
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		printf("client connected\n");
	
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			if(syst.getExitState())
			{
				close(newsockfd);
				while(true)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
				exit(0);
			}
			syst.engine_get(localEngine);
			syst.currentJPEG_get(lastjpeg);
			strToSnd.angle = localEngine.angle;
			strToSnd.speed = localEngine.real_speed;
			lastjpegSize = (uint32_t)lastjpeg.size();
			
			if(!send_data((char *)(&strToSnd), newsockfd, sizeof(EngineRelay))) goto sockerr;
			if(!send_data((char *)(&lastjpegSize), newsockfd, sizeof(uint32_t))) goto sockerr;
			if(!send_data((char *)(&lastjpeg[0]), newsockfd, lastjpegSize)) goto sockerr;
			
			if(false)
			{
				sockerr:
				printf("Restarting server...\n");
				close(newsockfd);
				break;
			}
		}
	}
}

#endif
#endif
