#include <iostream>
#include <conio.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVER "127.0.0.1"
#define PORT 9999
#define BUFFER_LENGTH 500

int main()
{
	SOCKET					serverSocket;
	sockaddr_in				server;
	int						serverInfoSize = sizeof(server);
	char					message[BUFFER_LENGTH];	
	WSADATA					wsa;

	cout << "Inicializando WinSock..." << endl;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cout << "Error inicializando WinSock. Codigo: " << WSAGetLastError() << endl;
		_getch();
		exit(EXIT_FAILURE);
	}

	cout << "Creando socket..." << endl;

	serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "No se pudo crear el socket." << endl;
		_getch();		
		exit(EXIT_FAILURE);
	}

	cout << "Inicializando socket UDP en el puerto " << PORT << endl;

	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = inet_addr(SERVER);
	server.sin_port = htons(PORT);

	while(true)
	{		
		cout << "Introduce un mensaje: ";
		cin >> message;

		if (sendto(serverSocket, message, strlen(message) , 0 , (sockaddr*) &server, serverInfoSize) == SOCKET_ERROR)
        {
            cout << "No se pudo crear el socket." << endl;
			_getch();		
			exit(EXIT_FAILURE);
        }
	}
}