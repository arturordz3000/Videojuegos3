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
#include <random>
#include <time.h>
#include "Util.h"
#include "Game.h"
#include "Camera.h"
#include "Structs.h"
#include "Particle.h"

using namespace std;

#pragma endregion

#define MAX_PARTICLE_VELOCITY 2
#define TO_RADIANS 3.141592 / 180

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
	ID3D11ShaderResourceView *colorMap;
	ID3D11SamplerState *colorMapSampler;
	ID3D11BlendState *alphaBlendState, *commonBlendState;
	
	ID3D11Device *device;
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
		deviceContext->IASetInputLayout( inputLayout );
		deviceContext->PSSetShaderResources(0, 1, &colorMap);
		deviceContext->PSSetSamplers(0, 1, &this->colorMapSampler);
		deviceContext->VSSetShader( vertexShader, 0, 0 );
		deviceContext->PSSetShader( pixelShader, 0, 0 );

		TurnOnAlphaBlending();
		for(vector<Particle>::iterator it = particles.begin(); it != particles.end(); it++)
			it->Draw();
		TurnOffAlphaBlending();
	}

	bool PrepareGraphicResources(ID3D11Device *device)
	{
		this->device = device;

		if ( !CompileShaders(device) || !CreateDirectXResources() )
			return false;

		return true;
	}

	void InstantiateParticle()
	{
		if (particles.size() > this->maxParticles) return;

		std::random_device rd;

		float maxAngle = 20 / 4294967295.0f;
		maxAngle = (rd() * maxAngle) + 80;
		maxAngle *= TO_RADIANS;


		float randomXVelocity = MAX_PARTICLE_VELOCITY * cos(maxAngle);
		float randomYVelocity = MAX_PARTICLE_VELOCITY * sin(maxAngle);

		float maxColor = 1.0f / 4294967295.0f;

		float r = maxColor * rd();
		float g = maxColor * rd();
		float b = maxColor * rd();

		Particle newParticle(this->deviceContext, this->position, 0.7, 
			XMFLOAT4(r, g, b, 1.0f), this->particleLifeSpan, XMFLOAT3(randomXVelocity, randomYVelocity, 0));
		
		if (newParticle.CreateDirectXResources(this->device))
			this->particles.push_back(newParticle);
	}

private:
	//Activa el alpha blend para dibujar con transparencias
	void TurnOnAlphaBlending()
	{
		float blendFactor[4];
		blendFactor[0] = 1.0f;
		blendFactor[1] = 1.0f;
		blendFactor[2] = 1.0f;
		blendFactor[3] = 1.0f;

		deviceContext->OMSetBlendState(alphaBlendState, blendFactor, 0xffffffff);
	}

	//Regresa al blend normal(solido)
	void TurnOffAlphaBlending()
	{
		deviceContext->OMSetBlendState(commonBlendState, NULL, 0xffffffff);
	}

	bool CreateDirectXResources()
	{
		HRESULT result = D3DX11CreateShaderResourceViewFromFile( device, L"smoke.png", 0, 0, &colorMap, 0 );
		if( FAILED(result) ) return false;

		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MinLOD = 0;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		result = device->CreateSamplerState( &colorMapDesc, &colorMapSampler );

		if( FAILED(result) ) return false;

		D3D11_BLEND_DESC descCBSD;
		ZeroMemory(&descCBSD, sizeof(D3D11_BLEND_DESC));
		descCBSD.RenderTarget[0].BlendEnable = FALSE;
		descCBSD.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		descCBSD.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		descCBSD.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		descCBSD.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		descCBSD.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		descCBSD.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		descCBSD.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		result = device->CreateBlendState(&descCBSD, &commonBlendState);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear el blend state", MB_OK);
			return false;
		}

		D3D11_BLEND_DESC descABSD;
		ZeroMemory(&descABSD, sizeof(D3D11_BLEND_DESC));
		descABSD.RenderTarget[0].BlendEnable = TRUE;
		descABSD.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		descABSD.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		descABSD.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		descABSD.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		descABSD.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		descABSD.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		descABSD.RenderTarget[0].RenderTargetWriteMask = 0x0f;

		result = device->CreateBlendState(&descABSD, &alphaBlendState);
		if(FAILED(result))
		{
			MessageBox(0, L"Error", L"Error al crear el blend state", MB_OK);
			return false;
		}

		return true;
	}

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
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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