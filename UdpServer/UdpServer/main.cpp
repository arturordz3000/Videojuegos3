#include <iostream>
#include <conio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 9999
#define BUFFER_LENGTH 500

int main()
{
	SOCKET					serverSocket;
	sockaddr_in				server;
	sockaddr_in				clientInfo;
	int						clientInfoSize = sizeof(clientInfo);
	char					buffer[BUFFER_LENGTH];	
	WSADATA					wsa;

	cout << "Inicializando WinSock..." << endl;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Error inicializando WinSock. Codigo: " << WSAGetLastError() << endl;
		_getch();
		exit(EXIT_FAILURE);
	}

	cout << "Creando socket..." << endl;

	serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "No se pudo crear el socket." << endl;
		_getch();		
		exit(EXIT_FAILURE);
	}

	cout << "Inicializando socket UDP en el puerto " << PORT << endl;

	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	int result = bind(serverSocket, (sockaddr*)&server, sizeof(server));
	if (result == SOCKET_ERROR)
	{
		cout << "Error inicializando socket UDP. Codigo: " << WSAGetLastError() << endl;
		_getch();
		exit(EXIT_FAILURE);
	}
	
	while (true)
	{
		cout << endl << "Esperando datos..." << endl;

		memset(buffer, '\0', BUFFER_LENGTH);

		int receivedLength = recvfrom(serverSocket, buffer, BUFFER_LENGTH, 0, (sockaddr*)&clientInfo, &clientInfoSize);
		if (receivedLength == SOCKET_ERROR)
		{
			cout << "Error recibiendo datos. Codigo: " << WSAGetLastError() << endl;
			_getch();
			exit(EXIT_FAILURE);
		}

		cout << "Datos recibidos!\nIP: " << inet_ntoa(clientInfo.sin_addr) << endl << "Port: " << ntohs(clientInfo.sin_port) << endl;
		cout << "Datos: " << buffer << endl;
	}

	return 0;
}