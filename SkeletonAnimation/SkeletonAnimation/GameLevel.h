#include <D3D11.h>
#include "Camera.h"
#include "Cube.h"
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
	Cube *cube;
	Camera *camera;

public:
	SimpleRenderLevel(ID3D11Device *device, ID3D11DeviceContext *deviceContext, bool *couldInitialize) : GameLevel(device)
	{
		mesh = new MD5Mesh("C:\\Model\\boy", deviceContext);
		//cube = new Cube();
		*couldInitialize = mesh->PrepareGraphicResources(this->_device);
		//*couldInitialize = cube->PrepareGraphicResources(this->_device);
		//if ( *couldInitialize ) *couldInitialize = cube->PrepareGraphicResources(this->_device);

		camera = new Camera(XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(3.0f, 3.0f, 3.0f), 800, 640);
	}

	~SimpleRenderLevel()
	{
		//delete mesh;
	}

	Camera* GetCamera() { return camera; }

	void Update(float deltaTime)
	{
		mesh->Update(deltaTime, camera);
		//cube->Update(deltaTime, camera);
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		mesh->Draw();
		//cube->Draw(deviceContext);
	}
};