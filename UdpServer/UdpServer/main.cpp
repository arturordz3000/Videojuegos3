#include <iostream>
#include <conio.h>
#include <WinSock2.h>
#include "GameServer.h"

using namespace std;

int main()
{
	g_GameServer = new GameServer();
	if (g_GameServer->Run())
	{
		while (true)
		{
			string sentMessage = "";

			cout << "Presiona una tecla para enviar informacion a clientes..." << endl;
			_getch();
			g_GameServer->SendDataToClients(&sentMessage);

			cout << endl << "Mensaje enviado: " + sentMessage << endl;
		}
	}

	return 0;
}