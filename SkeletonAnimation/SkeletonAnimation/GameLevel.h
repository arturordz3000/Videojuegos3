#pragma once

#include <D3D11.h>
#include "Camera.h"
#include "Cube.h"
#include "MD5Mesh.h"
#include "GameClient.h"
#include "Util.h"
#include "ParticleEmitter.h"
#include <string>

using namespace std;

#define SEND_INTERVAL 1.0
#define SERVER "127.0.0.1"
#define MAX_PARTICLES 2000
#define PARTICLE_LIFE_SPAN 1.5f
#define PARTICLE_INSTANTIATION_INTERVAL 0.01f

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
	GameClient *client;
	ParticleEmitter *particleEmitter;
	float sendDeltaTime;

	// Variables de prueba
	float mesh2DeltaTime;
	float mesh2Translation;

public:
	SimpleRenderLevel(ID3D11Device *device, ID3D11DeviceContext *deviceContext, bool *couldInitialize) : GameLevel(device)
	{
		g_SharedLevel = this;
		particleEmitter = new ParticleEmitter(deviceContext, XMFLOAT3(0, 0, 0), MAX_PARTICLES, 
			PARTICLE_INSTANTIATION_INTERVAL, PARTICLE_LIFE_SPAN); 

#pragma region Codigo no usado
		/*mesh2Translation = -3;
		mesh2DeltaTime = 0;

		sendDeltaTime = 0;

		client = new GameClient(SERVER);
		g_GameClient = client;
		client->Run(ReceiveData);
		mesh = new MD5Mesh("C:\\Model\\boy", deviceContext);
		mesh2 = new MD5Mesh("C:\\Model\\boy", deviceContext);

		*couldInitialize = mesh->PrepareGraphicResources(this->_device);
		*couldInitialize = mesh2->PrepareGraphicResources(this->_device);*/
#pragma endregion

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
		
		particleEmitter->Update(deltaTime, camera);
#pragma region Codigo no usado
		/*mesh->Update(deltaTime, camera, -3);
		mesh2->Update(mesh2DeltaTime, camera, mesh2Translation);

		ostringstream stringStream;
		stringStream << deltaTime << "," << mesh->translation.x << "," << mesh->translation.y << "," << mesh->translation.z;
		string message(stringStream.str());

		if (sendDeltaTime > SEND_INTERVAL)
		{
			client->SendDataToServer(message);
			sendDeltaTime = 0;
		}*/
#pragma endregion
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		particleEmitter->Draw();
#pragma region Codigo no usado
		/*mesh->Draw();
		mesh2->Draw();*/
#pragma endregion
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
	while (g_GameClient->canRun)
	{
		g_GameClient->ReceiveData();
		g_SharedLevel->UpdatePlayersData(g_GameClient->clientsData);
	}

	return 0;
}