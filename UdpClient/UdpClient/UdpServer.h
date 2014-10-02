#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_LENGTH 500

class UdpServer
{
private:
	SOCKET					serverSocket;
	sockaddr_in				serverInfo;
	sockaddr_in				clientInfo;
	int						clientInfoSize;
	WSADATA					wsa;
	int						port;

public:
	UdpServer(int *initResult)
	{		
		*initResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		clientInfoSize = sizeof(sockaddr_in);
	}

	bool Bind(int port)
	{
		this->port = port;

		serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
		if (serverSocket == INVALID_SOCKET)
			return false;

		serverInfo.sin_family = AF_INET;
		serverInfo.sin_addr.S_un.S_addr = INADDR_ANY;
		serverInfo.sin_port = htons(port);

		int result = bind(serverSocket, (sockaddr*)&serverInfo, sizeof(serverInfo));
		if (result == SOCKET_ERROR)
			return false;

		return true;
	}

	int WaitForData(char *buffer)
	{
		memset(buffer, '\0', BUFFER_LENGTH);

		int receivedLength = recvfrom(serverSocket, buffer, BUFFER_LENGTH, 0, (sockaddr*)&clientInfo, &clientInfoSize);
		if (receivedLength == SOCKET_ERROR)
			return -1;
		else
			return receivedLength;
	}

	int WaitForData(char *buffer, sockaddr_in &pClientInfo)
	{
		memset(buffer, '\0', BUFFER_LENGTH);

		int receivedLength = recvfrom(serverSocket, buffer, BUFFER_LENGTH, 0, (sockaddr*)&clientInfo, &clientInfoSize);
		if (receivedLength == SOCKET_ERROR)
		{
			return -1;
		}
		else
		{
			pClientInfo = clientInfo;
			return receivedLength;
		}
	}
};

#endif