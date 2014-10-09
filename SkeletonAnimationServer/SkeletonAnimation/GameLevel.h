#pragma once

#include <D3D11.h>
#include "Camera.h"
#include "Cube.h"
#include "MD5Mesh.h"
#include "GameServer.h"
#include "Util.h"
#include <string>

using namespace std;

#define SEND_INTERVAL 1.0

class SimpleRenderLevel;
DWORD WINAPI ReceiveData(LPVOID lpParam);
SimpleRenderLevel *g_SharedLevel = NULL;

class GameLevel
{
protected:
	ID3D11Device *_device;

public:
	GameLevel(ID3D11Device *device) { _device = device; }
	virtual void Update(float deltaTime){}
	virtual void Draw(ID3D11DeviceContext *deviceContext){}
};

class SimpleRenderLevel :
	public GameLevel
{
private:
	MD5Mesh *mesh;
	MD5Mesh *mesh2;
	Cube *cube;
	Camera *camera;
	GameServer *server;
	float sendDeltaTime;

	// Variables de prueba
	float mesh2DeltaTime;
	float mesh2Translation;

public:
	SimpleRenderLevel(ID3D11Device *device, ID3D11DeviceContext *deviceContext, bool *couldInitialize) : GameLevel(device)
	{
		g_SharedLevel = this;

		mesh2Translation = -3;
		mesh2DeltaTime = 0;

		sendDeltaTime = 0;

		server = new GameServer();
		g_GameServer = server;
		server->Run(ReceiveData);

		mesh = new MD5Mesh("C:\\Model\\boy", deviceContext);
		mesh2 = new MD5Mesh("C:\\Model\\boy", deviceContext);

		*couldInitialize = mesh->PrepareGraphicResources(this->_device);
		*couldInitialize = mesh2->PrepareGraphicResources(this->_device);

		camera = new Camera(XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(3.0f, 3.0f, 3.0f), 800, 640);
	}

	~SimpleRenderLevel()
	{
		//delete mesh;
	}

	Camera* GetCamera() { return camera; }

	void Update(float deltaTime)
	{
		sendDeltaTime += deltaTime;

		mesh->Update(deltaTime, camera, -3);
		mesh2->Update(mesh2DeltaTime, camera, mesh2Translation);

		ostringstream stringStream;
		stringStream << deltaTime << "," << mesh->translation.x << "," << mesh->translation.y << "," << mesh->translation.z;
		string message(stringStream.str());

		if (sendDeltaTime > SEND_INTERVAL)
		{
			string sentMessage;
			server->SendDataToClients(message);
			sendDeltaTime = 0;
		}
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		mesh->Draw();
		mesh2->Draw();
	}

	void UpdatePlayersData(map<string, string> clientsData)
	{
		if (clientsData.size() > 0)
		{
			map<string, string>::iterator it = clientsData.begin();
			vector<string> secondPlayerData = SplitString(it->second, ",");
			mesh2DeltaTime = stof(secondPlayerData[0]);
			mesh2Translation = stof(secondPlayerData[1]);
		}
	}	
};

DWORD WINAPI ReceiveData(LPVOID lpParam)
{
	while (g_GameServer->canRun)
	{
		g_GameServer->ReceiveData();
		g_SharedLevel->UpdatePlayersData(g_GameServer->clientsData);
	}

	return 0;
}