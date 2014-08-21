#include <D3D11.h>
#include "Camera.h"
#include "MD5Mesh.h"
#include "Cube.h"

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
	Cube *cube;
	Camera *camera;

public:
	SimpleRenderLevel(ID3D11Device *device, bool *couldInitialize) : GameLevel(device)
	{
		camera = new Camera(XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(3.0f, 3.0f, 3.0f), 800, 640);
		mesh = new MD5Mesh("C:\\Model\\boy.md5mesh");
	}

	~SimpleRenderLevel()
	{
		
	}

	Camera* GetCamera() { return camera; }

	void Update(float deltaTime)
	{

	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{

	}
};