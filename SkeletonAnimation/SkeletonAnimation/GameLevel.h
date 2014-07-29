#include <D3D11.h>
#include "Camera.h"
#include "MD5Mesh.h"

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
	Camera *camera;

public:
	SimpleRenderLevel(ID3D11Device *device) : GameLevel(device)
	{
		mesh = new MD5Mesh("C:\\Model\\bob_lamp_update.md5mesh");
		mesh->PrepareGraphicResources(this->_device);

		camera = new Camera(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(.05f, .05f, .05f), 800, 640);
	}

	~SimpleRenderLevel()
	{
		delete mesh;
	}

	void Update(float deltaTime)
	{
		mesh->Update(deltaTime, camera);
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		mesh->Draw(deviceContext);
	}
};