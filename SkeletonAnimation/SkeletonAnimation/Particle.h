#ifndef _PARTICLE_H_INCLUDED
#define _PARTICLE_H_INCLUDED

#pragma region Includes

#include <iostream>
#include <fstream>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>
#include "Util.h"
#include "Game.h"
#include "Camera.h"
#include "Structs.h"

#pragma endregion

class Particle
{
private:
	XMFLOAT3			position;
	float				size;
	XMFLOAT4			color;
	float				lifeSpan;
	float				timeAlive;
	XMFLOAT3			velocity;
	ID3D11DeviceContext *deviceContext;

public:
	Particle(ID3D11DeviceContext *deviceContext, 
		XMFLOAT3 position, float size, XMFLOAT4 color, float lifeSpan, XMFLOAT3 velocity)
	{
		this->position = position;
		this->size = size;
		this->color = color;
		this->lifeSpan = lifeSpan;
		this->velocity = velocity;
		this->deviceContext = deviceContext;
		this->timeAlive = 0;
	}

	void Update(float deltaTime, Camera *camera)
	{
		timeAlive += deltaTime;
		this->position.x += this->velocity.x;
		this->position.y += this->velocity.y;
		this->position.z += this->velocity.z;
	}

	void Draw()
	{

	}

	bool IsAlive()
	{
		return timeAlive <= lifeSpan;
	}
};

#endif