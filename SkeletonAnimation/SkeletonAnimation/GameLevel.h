#include <D3D11.h>
#include "Camera.h"

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
	Camera *camera;

public:
	SimpleRenderLevel(ID3D11Device *device) : GameLevel(device)
	{
		camera = new Camera(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(.05f, .05f, .05f), 800, 640);
	}

	~SimpleRenderLevel()
	{

	}

	void Update(float deltaTime)
	{
		
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		
	}
};