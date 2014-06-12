#include <D3D11.h>

class GameLevel
{
public:
	virtual void Update(float deltaTime){}
	virtual void Draw(ID3D11DeviceContext *deviceContext){}
};

class SimpleRenderLevel :
	public GameLevel
{
public:
	SimpleRenderLevel()
	{
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