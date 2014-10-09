#ifndef _UDP_GAMECLIENT_H_
#define _UDP_GAMECLIENT_H_

#include <iostream>
#include <string>
#include "UdpServer.h"
#include "UdpClient.h"
#include <map>
#include <Windows.h>

using namespace std;

DWORD WINAPI ThreadFunction(LPVOID lpParam);

class GameClient
{
	
private:
	UdpServer *server;
	UdpClient *sender;

	LPDWORD dwThreadID;
	HANDLE hThread;

	string serverIp;

public:
	bool canRun;
	map<string, string> clientsData;

	GameClient(string serverIp)
	{
		canRun = true;
		this->serverIp = serverIp;

		int initResult = 0;

		server = new UdpServer(&initResult);
		if (initResult != 0) canRun = false;

		sender = new UdpClient(&initResult);
		if (initResult != 0) canRun = false;
	}

	bool Run()
	{
		if (!canRun) return false;
		if (!server->Bind(9998)) return false;
		if (!sender->Bind((char*)this->serverIp.c_str(), 9999)) return false;

		dwThreadID = 0;
		hThread = CreateThread(NULL, 0, ThreadFunction, NULL, 0, dwThreadID);
	}

	bool Run(LPTHREAD_START_ROUTINE receivedDataDelegate)
	{
		if (!canRun) return false;
		if (!server->Bind(9998)) return false;
		if (!sender->Bind((char*)this->serverIp.c_str(), 9999)) return false;

		dwThreadID = 0;
		hThread = CreateThread(NULL, 0, receivedDataDelegate, NULL, 0, dwThreadID);
	}

	void Stop()
	{
		canRun = false;
	}

	int ReceiveData()
	{
		char buffer[BUFFER_LENGTH];
		sockaddr_in clientInfo;
		int receivedData = server->WaitForData(buffer, clientInfo);

		if (receivedData == -1) return receivedData;

		string clientAddress = string(inet_ntoa(clientInfo.sin_addr));
		string data = string(buffer);

		map<string, string>::iterator it = clientsData.find(clientAddress);
		if (it == clientsData.end())
		{
			// No encontrado, insertamos en el diccionario
			clientsData.insert(make_pair(clientAddress, data));
		}
		else
			it->second = data;

		return receivedData;
	}

	bool SendDataToServer(string sentMessage)
	{		
		return sender->SendMessageToEndPoint((char*)sentMessage.c_str(), sentMessage.length());
	}
};

GameClient *g_GameClient = NULL;

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	while (g_GameClient->canRun)
		g_GameClient->ReceiveData();

	return 0;
}

#endif