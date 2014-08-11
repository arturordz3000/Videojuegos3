#ifndef _CUBE_H_INCLUDED
#define _CUBE_H_INCLUDED

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

#pragma endregion

#pragma region Namespaces

using namespace std;

#pragma endregion

#pragma region Substructures

struct VertexCube
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
};

struct MatrixBufferCube
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

#pragma endregion

class Cube
{
#pragma region Private members

private:

	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *constantBuffer;
	ID3D11SamplerState *colorMapSampler;

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11ShaderResourceView *colorMap;

	XMFLOAT3 translation;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMMATRIX world;

	MatrixBufferCube matrixBuffer;

#pragma endregion

#pragma region Public methods

public:
	Cube()
	{
	}

	~Cube()
	{
		
	}

	bool CompileShaders(ID3D11Device *device)
	{
		ID3DBlob *vertexShaderBlob;
		ID3DBlob *pixelShaderBlob;
		HRESULT d3dResult;
		bool compileResult;

		// Compilando vertex shader
		compileResult = CompileD3DShader("Shaders\\CubeShader.fx", "VS_Main", "vs_4_0", &vertexShaderBlob);
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
		compileResult = CompileD3DShader("Shaders\\CubeShader.fx", "PS_Main", "ps_4_0", &pixelShaderBlob);
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
		
		this->world = XMMatrixIdentity();

		return true;
	}

	bool PrepareGraphicResources(ID3D11Device *device)
	{
		if ( !CompileShaders(device) )
			return false;
		if ( !CreateDirectXResources(device) )
			return false;

		return true;
	}

	void Update(float deltaTime, Camera *camera)
	{
		static float rotation;
		rotation += .001f;

		this->world = XMMatrixRotationY( rotation ) * XMMatrixRotationX( rotation ) * XMMatrixTranslation(-2.0f, 0.0f, 0.0f);
		XMVECTOR Eye = XMVectorSet( 0.0f, 3.0f, -6.0f, 0.0f );
		XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		XMMATRIX viewMatrix = XMMatrixLookAtLH( Eye, At, Up );
		XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV4, 784 / (FLOAT)562, 0.01f, 100.0f );

		//this->world = XMMatrixTranslation(0, 0, 0) *  XMMatrixScaling( 1.0f, 1.0f, 1.0f );
		matrixBuffer.world		= XMMatrixTranspose( this->world );
		matrixBuffer.view		= XMMatrixTranspose( viewMatrix );
		matrixBuffer.projection = XMMatrixTranspose( projectionMatrix );
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		deviceContext->IASetInputLayout( this->inputLayout );
		deviceContext->VSSetShader( this->vertexShader, NULL, 0 );
		deviceContext->PSSetShader( this->pixelShader, NULL, 0 );
		deviceContext->UpdateSubresource( this->constantBuffer, 0, 0, &this->matrixBuffer, sizeof(MatrixBufferCube), 0 );
		deviceContext->VSSetConstantBuffers( 0, 1, &this->constantBuffer );
		deviceContext->PSSetSamplers( 0, 1, &this->colorMapSampler );

		UINT uiStride = sizeof (VertexCube);
		UINT uiOffset = 0;
			
		deviceContext->PSSetShaderResources( 0, 1, &colorMap );
		deviceContext->IASetVertexBuffers( 0, 1,  &vertexBuffer, &uiStride, &uiOffset );
		deviceContext->IASetIndexBuffer( indexBuffer, DXGI_FORMAT_R16_UINT, 0 );
		deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );	
		deviceContext->DrawIndexed( 36, 0, 0 );
	}

#pragma endregion

#pragma region Private methods

private:
	bool CreateDirectXResources(ID3D11Device *device)
	{
		HRESULT result;

		WORD indices[] =
		{
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
		};

		// Creamos el index buffer
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		//indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = indices;
		result = device->CreateBuffer(&indexBufferDesc, &iinitData, &indexBuffer);

		if ( FAILED(result) ) return false;

		VertexCube vertices[] =
		{
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

			{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

			{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
			{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
			{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
		};

		//Creamos el vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;							// We will be updating this buffer, so we must set as dynamic
		vertexBufferDesc.ByteWidth = sizeof( VertexCube ) * ARRAYSIZE(vertices);
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;				// Give CPU power to write to buffer
		//vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData; 
		ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
		vertexBufferData.pSysMem = vertices;
		result = device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &vertexBuffer);

		if ( FAILED(result) ) return false;			

		result = D3DX11CreateShaderResourceViewFromFile( device, "p.jpg", 0, 0, &colorMap, 0 );

		if( FAILED(result) ) return false;		

		D3D11_BUFFER_DESC d3dBufferDescriptor;
		ZeroMemory( &d3dBufferDescriptor, sizeof(d3dBufferDescriptor) );

		// Creamos el constant buffer
		d3dBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
		d3dBufferDescriptor.ByteWidth = sizeof(MatrixBufferCube);
		d3dBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		d3dBufferDescriptor.CPUAccessFlags = 0;
		result = device->CreateBuffer( &d3dBufferDescriptor, NULL, &this->constantBuffer );

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

		return true;
	}

#pragma endregion
};

#endif