#include <D3D11.h>
#include "Camera.h"
#include "Cube.h"
#include "MD5Mesh.h"
#include "UdpClient.h"
#include <string>

using namespace std;

#define SEND_INTERVAL 1.0

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
	UdpClient *client;
	float sendDeltaTime;

public:
	SimpleRenderLevel(ID3D11Device *device, ID3D11DeviceContext *deviceContext, bool *couldInitialize) : GameLevel(device)
	{
		sendDeltaTime = 0;
		int initResult = 0;
		client = new UdpClient(&initResult);
		client->Bind("127.0.0.1", 9999);

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
		mesh2->Update(deltaTime, camera, 3);

		ostringstream stringStream;
		stringStream << deltaTime << "," << mesh->translation.x << "," << mesh->translation.y << "," << mesh->translation.z;
		string message(stringStream.str());

		if (sendDeltaTime > SEND_INTERVAL)
		{
			client->SendMessageToEndPoint((char*)message.c_str(), message.length());
			sendDeltaTime = 0;
		}

		//cube->Update(deltaTime, camera);
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		mesh->Draw();
		mesh2->Draw();
		//cube->Draw(deviceContext);
	}
};