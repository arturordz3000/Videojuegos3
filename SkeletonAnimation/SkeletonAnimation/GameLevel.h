#include <D3D11.h>
#include "MD5Mesh.h"
#include "Camera.h"

class GameLevel
{
public:
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
	SimpleRenderLevel()
	{
		mesh = new MD5Mesh("C:\\Model\\bob_lamp_update.md5mesh");
		camera = new Camera(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(.05f, .05f, .05f));
	}

	~SimpleRenderLevel()
	{
		delete mesh;
	}

	void Update(float deltaTime)
	{
		mesh->Update(deltaTime);
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		mesh->Draw(deviceContext);
	}
};