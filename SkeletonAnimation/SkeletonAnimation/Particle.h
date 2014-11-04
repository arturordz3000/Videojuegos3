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
	float				halfSize;
	XMFLOAT4			color;
	float				lifeSpan;
	float				timeAlive;
	XMFLOAT3			velocity;
	ID3D11DeviceContext *deviceContext;
	ID3D11Buffer*		vertexBuffer;
	ID3D11Buffer*		matrixBufferCB;
	MatrixBuffer*		matrices; 

public:
	Particle(ID3D11DeviceContext *deviceContext, 
		XMFLOAT3 position, float size, XMFLOAT4 color, float lifeSpan, XMFLOAT3 velocity)
	{
		this->position = position;
		this->size = size;
		this->halfSize = size / 2.0f;
		this->color = color;
		this->lifeSpan = lifeSpan;
		this->velocity = velocity;
		this->deviceContext = deviceContext;
		this->timeAlive = 0;
	}

	bool CreateDirectXResources(ID3D11Device *d3dDevice)
	{
		HRESULT d3dResult;

		ParticleComponent vertices[] =
		{
			{ XMFLOAT3(-halfSize, -halfSize, 0.0f), color },
			{ XMFLOAT3(-halfSize, halfSize, 0.0f), color },
			{ XMFLOAT3(halfSize, -halfSize, 0.0f), color },
			{ XMFLOAT3(halfSize, halfSize, 0.0f), color },
		};

		D3D11_BUFFER_DESC vertexDesc;
		ZeroMemory( &vertexDesc, sizeof( vertexDesc ) );
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.ByteWidth = sizeof( ParticleComponent ) * 4;

		D3D11_SUBRESOURCE_DATA resourceData;
		ZeroMemory( &resourceData, sizeof( resourceData ) );
		resourceData.pSysMem = vertices;

		d3dResult = d3dDevice->CreateBuffer( &vertexDesc, &resourceData, &vertexBuffer );

		if( FAILED( d3dResult ) )
		{
			MessageBox(0, L"Error", L"Error al crear vertex buffer", MB_OK);
			return false;
		}

		D3D11_BUFFER_DESC constDesc;
		ZeroMemory( &constDesc, sizeof( constDesc ) );
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = sizeof( MatrixBuffer );
		constDesc.Usage = D3D11_USAGE_DEFAULT;

		d3dResult = d3dDevice->CreateBuffer( &constDesc, 0, &matrixBufferCB );

		if( FAILED( d3dResult ) )
		{
			MessageBox(0, L"Error", L"Error al crear constant buffer", MB_OK);
			return false;
		}
		matrices = new MatrixBuffer;
	}

	void Update(float deltaTime, Camera *camera)
	{
		timeAlive += deltaTime;
		this->position.x += this->velocity.x * deltaTime;
		this->position.y += this->velocity.y * deltaTime;
		this->position.z += this->velocity.z * deltaTime;
		
		XMMATRIX translate;
		translate = XMMatrixTranslation(this->position.x, this->position.y, this->position.z);

		matrices->world = XMMatrixTranspose(translate);
		matrices->view = camera->GetViewMatrix();
		matrices->projection = camera->GetProjectionMatrix();
	}

	void Draw()
	{
		unsigned int stride = sizeof( ParticleComponent );
		unsigned int offset = 0;

		deviceContext->IASetVertexBuffers( 0, 1, &vertexBuffer, &stride, &offset );
		deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

		deviceContext->UpdateSubresource( matrixBufferCB, 0, 0, matrices, sizeof(MatrixBuffer), 0 );
		deviceContext->VSSetConstantBuffers( 0, 1, &matrixBufferCB );

		deviceContext->Draw(4,0);
	}

	bool IsAlive()
	{
		return timeAlive <= lifeSpan;
	}
};

#endif