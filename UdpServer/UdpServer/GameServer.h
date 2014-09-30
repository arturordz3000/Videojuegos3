#ifndef _UDP_GAMESERVER_H_
#define _UDP_GAMESERVER_H_

#include <iostream>
#include <string>
#include "UdpServer.h"
#include "UdpClient.h"
#include <map>
#include <Windows.h>

using namespace std;

#define PORT 9999
#define BUFFER_LENGTH 500

DWORD WINAPI ThreadFunction(LPVOID lpParam);

class GameServer
{
private:
	UdpServer *server;
	UdpClient *sender;
	map<string, string> clientsData;

	LPDWORD dwThreadID;
	HANDLE hThread;

public:
	bool canRun;

	GameServer()
	{
		canRun = true;

		int initResult = 0;

		server = new UdpServer(&initResult);
		if (initResult != 0) canRun = false;

		sender = new UdpClient(&initResult);
		if (initResult != 0) canRun = false;
	}

	bool Run()
	{
		if (!canRun) return false;
		if (!server->Bind(PORT)) return false;
		if (!sender->Bind(NULL, 9998)) return false;

		dwThreadID = 0;
		hThread = CreateThread(NULL, 0, ThreadFunction, NULL, 0, dwThreadID);
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

	bool SendDataToClients(string *sentMessage)
	{
		for (map<string, string>::iterator it = clientsData.begin(); it != clientsData.end(); it++)
			*sentMessage += it->first + "," + it->second + "|";

		return sender->SendMessageToEndPoint((char*)sentMessage->c_str(), sentMessage->length());
	}
};

GameServer *g_GameServer = NULL;

DWORD WINAPI ThreadFunction(LPVOID lpParam)
{
	while (g_GameServer->canRun)
		g_GameServer->ReceiveData();

	return 0;
}

#endif