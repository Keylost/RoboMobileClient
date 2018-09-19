#if defined(RELAYING_MODE)
#include "server.hpp"

void server_fnc(System &syst)
{
	Engine localEngine;
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
		hints.ai_family = AF_INET;
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
			syst.engine_get(localEngine);
			srtTosnd.angle = localEngine.angle;
			strToSnd.speed = localEngine.real_speed;

			iSendResult = send(ClientSocket, (char *)(&strToSnd), sizeof(EngineRelay), 0);
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

#endif
