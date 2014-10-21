#ifndef _PARTICLE_EMITTER_H_INCLUDED
#define _PARTICLE_EMITTER_H_INCLUDED

#pragma region Includes

#include <iostream>
#include <fstream>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>
#include <time.h>
#include "Util.h"
#include "Game.h"
#include "Camera.h"
#include "Structs.h"
#include "Particle.h"

using namespace std;

#pragma endregion

#define MAX_PARTICLE_VELOCITY 5

class ParticleEmitter
{
private:
	XMFLOAT3			position;
	int					maxParticles;
	float				instantiationInterval;
	float				particleLifeSpan;
	vector<Particle>	particles;

	float instantiationElapsedTime;

	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader*	pixelShader;

	ID3D11InputLayout*	inputLayout;
	ID3D11Buffer*		vertexBuffer;
	
	ID3D11DeviceContext *deviceContext;

public:
	ParticleEmitter(ID3D11DeviceContext *deviceContext, 
		XMFLOAT3 position, int maxParticles, float instantiationInterval, float particleLifeSpan)
	{
		this->deviceContext = deviceContext;
		this->position = position;
		this->maxParticles = maxParticles;
		this->instantiationInterval = instantiationInterval;
		this->particleLifeSpan = particleLifeSpan;
		this->instantiationElapsedTime = 0;
	}

	void Update(float deltaTime, Camera *camera)
	{
		instantiationElapsedTime += deltaTime;
		if (instantiationElapsedTime >= instantiationInterval)
		{
			InstantiateParticle();
			instantiationElapsedTime = 0;
		}

		for(vector<Particle>::iterator it = particles.begin(); it != particles.end(); it++)
		{
			if (!it->IsAlive())
			{
				it = particles.erase(it);

				if (it == particles.end()) 
					break;
				else
					continue;
			}

			it->Update(deltaTime, camera);
		}
	}

	void Draw()
	{		
		for(vector<Particle>::iterator it = particles.begin(); it != particles.end(); it++)
			it->Draw();
	}

	bool PrepareGraphicResources(ID3D11Device *device)
	{
		if ( !CompileShaders(device) )
			return false;

		return true;
	}

	void InstantiateParticle()
	{
		if (particles.size() > this->maxParticles) return;

		srand(time(NULL));
		float randomXVelocity = rand() % MAX_PARTICLE_VELOCITY;
		srand(time(NULL));
		float randomYVelocity = rand() % MAX_PARTICLE_VELOCITY;

		srand(time(NULL));
		float r = 1.0f / (rand() % 255);
		srand(time(NULL));
		float g = 1.0f / (rand() % 255);
		srand(time(NULL));
		float b = 1.0f / (rand() % 255);

		Particle newParticle(this->deviceContext, this->position, 1, 
			XMFLOAT4(r, g, b, 1.0f), this->particleLifeSpan, XMFLOAT3(randomXVelocity, randomYVelocity, 0));

		this->particles.push_back(newParticle);
	}

private:
	bool CompileShaders(ID3D11Device *device)
	{
		ID3DBlob *vertexShaderBlob;
		ID3DBlob *pixelShaderBlob;
		HRESULT d3dResult;
		bool compileResult;

		// Compilando vertex shader
		compileResult = CompileD3DShader(L"ParticleShader.fx", "VS_Main", "vs_4_0", &vertexShaderBlob);
		if ( !compileResult )
			return false;

		d3dResult = device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), 
											   vertexShaderBlob->GetBufferSize(),
											   0, 
											   &vertexShader);
		if ( FAILED(d3dResult) )
		{
			if ( vertexShaderBlob )
				vertexShaderBlob->Release();

			return false;
		}
		
		// Creando el input layout
		D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		unsigned int totalLayoutElements = ARRAYSIZE(solidColorLayout);
		d3dResult = device->CreateInputLayout(solidColorLayout, 
											  totalLayoutElements,
											  vertexShaderBlob->GetBufferPointer( ), 
											  vertexShaderBlob->GetBufferSize( ),
											  &inputLayout );

		if ( FAILED(d3dResult) )
		{
			if ( vertexShaderBlob )
				vertexShaderBlob->Release();

			return false;
		}

		// Compilando pixel shader
		compileResult = CompileD3DShader(L"ParticleShader.fx", "PS_Main", "ps_4_0", &pixelShaderBlob);
		if ( !compileResult )
			return false;

		d3dResult = device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), 
											   pixelShaderBlob->GetBufferSize(),
											   0, 
											   &pixelShader);
		if ( FAILED(d3dResult) )
		{
			if ( pixelShaderBlob )
				pixelShaderBlob->Release();

			return false;
		}

		return true;
	}
};

#endif