#ifndef _UDP_CLIENT_H_
#define _UDP_CLIENT_H_

#include <WinSock2.h>

class UdpClient
{
private:
	SOCKET					endPointSocket;
	sockaddr_in				endPoint;
	int						endPointInfoSize;
	WSADATA					wsa;
	char *address;
	int port;
	BOOL bOptVal;
    int bOptLen;

public:
	UdpClient(int *initResult)
	{
		*initResult = WSAStartup(MAKEWORD(2, 2), &wsa);
		endPointInfoSize = sizeof(sockaddr_in);
		bOptVal = TRUE;
		bOptLen = sizeof (BOOL);
	}

	bool Bind(char *address, int port)
	{
		this->address = address;
		this->port = port;

		endPointSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (endPointSocket == INVALID_SOCKET)
			return false;

		unsigned long sAddr = address == NULL ? INADDR_BROADCAST : inet_addr(address);

		endPoint.sin_family = AF_INET;
		endPoint.sin_addr.S_un.S_addr = sAddr;
		endPoint.sin_port = htons(port);
		
		if (sAddr == INADDR_BROADCAST)
			setsockopt(endPointSocket, SOL_SOCKET, SO_BROADCAST, (char *) &bOptVal, bOptLen);

		return true;
	}

	bool SendMessageToEndPoint(char *message, int messageLength)
	{
		if (sendto(endPointSocket, message, strlen(message) , 0 , (sockaddr*) &endPoint, endPointInfoSize) == SOCKET_ERROR)
			return false;
		else return true;
	}
};

#endif