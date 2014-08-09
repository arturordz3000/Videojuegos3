#ifndef _MESH_H_INCLUDED
#define _MESH_H_INCLUDED

#pragma region Includes

#include <iostream>
#include <fstream>
#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include "Util.h"
#include "Game.h"
#include "Camera.h"

#pragma endregion

#pragma region Namespaces

using namespace std;

#pragma endregion

#pragma region Substructures

struct Joint
{
	string name;
	int parent;
	XMFLOAT3 position;
	XMFLOAT4 orientation;
};

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;

	int vertexIndex;
	int startWeight;
	int countWeight;

	int timesUsed;
};

struct Triangle
{
	int triangleIndex;
	int vertexIndices[3];
};

struct Weight
{
	int weightIndex;
	int joint;
	float bias;
	XMFLOAT3 position;
};

struct Mesh
{
	string shader;
	int numVertices;
	int numTriangles;
	int numWeights;
	Vertex *vertices;
	Triangle *triangles;
	Weight *weights;
	int *indices;

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11ShaderResourceView *colorMap;

	~Mesh()
	{
		delete[] vertices;
		delete[] triangles;
		delete[] weights;
	}
};

struct MatrixBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

#pragma endregion

class MD5Mesh
{
#pragma region Private members

private:
	string filename;

	int numJoints;
	int numMeshes;
	Joint *joints;
	Mesh *meshes;

	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *constantBuffer;
	ID3D11SamplerState *colorMapSampler;

	XMFLOAT3 translation;
	XMFLOAT3 rotation;
	XMFLOAT3 scale;
	XMMATRIX world;

	MatrixBuffer matrixBuffer;

#pragma endregion

#pragma region Public methods

public:
	MD5Mesh(string filename)
	{
		this->filename = filename;

		ifstream fileStream(filename, ifstream::in);
		
		if (fileStream.is_open())
		{
			ReadNumJointsAndMeshes(fileStream);
			ReadJoints(fileStream);
			ReadMeshes(fileStream);
		}
	}

	~MD5Mesh()
	{
		delete[] joints;
		delete[] meshes;
	}

	bool CompileShaders(ID3D11Device *device)
	{
		ID3DBlob *vertexShaderBlob;
		ID3DBlob *pixelShaderBlob;
		HRESULT d3dResult;
		bool compileResult;

		// Compilando vertex shader
		compileResult = CompileD3DShader("Shaders\\TestShader.fx", "VS_Main", "vs_4_0", &vertexShaderBlob);
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
			{ "NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
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
		compileResult = CompileD3DShader("Shaders\\TestShader.fx", "PS_Main", "ps_4_0", &pixelShaderBlob);
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
		this->world = XMMatrixTranslation(0, 0, 2);
		matrixBuffer.world		= XMMatrixTranspose( this->world );
		matrixBuffer.view		= camera->GetViewMatrix();
		matrixBuffer.projection = camera->GetProjectionMatrix();
	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{
		deviceContext->IASetInputLayout( this->inputLayout );
		deviceContext->VSSetShader( this->vertexShader, NULL, 0 );
		deviceContext->PSSetShader( this->pixelShader, NULL, 0 );
		deviceContext->UpdateSubresource( this->constantBuffer, 0, 0, &this->matrixBuffer, sizeof(MatrixBuffer), 0 );
		deviceContext->VSSetConstantBuffers( 0, 1, &this->constantBuffer );
		deviceContext->PSSetSamplers( 0, 1, &this->colorMapSampler );

		UINT uiStride = sizeof (Vertex) - 4;
		UINT uiOffset = 0;

		for (int i = 0; i < numMeshes; i++)
		{
			Mesh *currentMesh = &meshes[i];
			
			deviceContext->PSSetShaderResources( 0, 1, &currentMesh->colorMap );
			deviceContext->IASetVertexBuffers( 0, 1,  &currentMesh->vertexBuffer, &uiStride, &uiOffset );
			deviceContext->IASetIndexBuffer( currentMesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0 );
			deviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );	
			deviceContext->DrawIndexed( currentMesh->numTriangles * 3, 0, 0 );
		}
	}

#pragma endregion

#pragma region Private methods

private:
	bool CreateDirectXResources(ID3D11Device *device)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			Mesh *currentMesh = &meshes[i];
			ComputeVerticesPositions(currentMesh);
			ComputeNormals(currentMesh);

			HRESULT result;

			// Creamos el index buffer
			D3D11_BUFFER_DESC indexBufferDesc;
			ZeroMemory( &indexBufferDesc, sizeof(indexBufferDesc) );

			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(int) * currentMesh->numTriangles * 3;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA iinitData;
			iinitData.pSysMem = currentMesh->indices;
			result = device->CreateBuffer(&indexBufferDesc, &iinitData, &currentMesh->indexBuffer);

			if ( FAILED(result) ) return false;

			//Creamos el vertex buffer
			D3D11_BUFFER_DESC vertexBufferDesc;
			ZeroMemory( &vertexBufferDesc, sizeof(vertexBufferDesc) );

			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;							// We will be updating this buffer, so we must set as dynamic
			vertexBufferDesc.ByteWidth = sizeof( Vertex ) * currentMesh->numVertices;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// Give CPU power to write to buffer
			vertexBufferDesc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA vertexBufferData; 
			ZeroMemory( &vertexBufferData, sizeof(vertexBufferData) );
			vertexBufferData.pSysMem = currentMesh->vertices;
			result = device->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &currentMesh->vertexBuffer);

			if ( FAILED(result) ) return false;			

			result = D3DX11CreateShaderResourceViewFromFile( device, ("C:\\Model\\" + currentMesh->shader).c_str(), 0, 0, &currentMesh->colorMap, 0 );

			if( FAILED(result) ) return false;
		}

		D3D11_BUFFER_DESC d3dBufferDescriptor;
		ZeroMemory( &d3dBufferDescriptor, sizeof(d3dBufferDescriptor) );

		// Creamos el constant buffer
		d3dBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
		d3dBufferDescriptor.ByteWidth = sizeof(MatrixBuffer);
		d3dBufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		d3dBufferDescriptor.CPUAccessFlags = 0;
		HRESULT result = device->CreateBuffer( &d3dBufferDescriptor, NULL, &this->constantBuffer );

		if( FAILED(result) ) return false;

		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory( &colorMapDesc, sizeof( colorMapDesc ) );
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		result = device->CreateSamplerState( &colorMapDesc, &colorMapSampler );

		if( FAILED(result) ) return false;

		return true;
	}

	void ComputeVerticesPositions(Mesh *currentMesh)
	{
		for (int j = 0; j < currentMesh->numVertices; j++)
		{
			Vertex *currentVertex = &currentMesh->vertices[j];
			currentVertex->position = XMFLOAT3(0, 0, 0);
			currentVertex->normal	= XMFLOAT3(0, 0, 0);
			currentVertex->tangent	= XMFLOAT3(0, 0, 0);
			currentVertex->timesUsed = 0;

			for (int k = 0; k < currentVertex->countWeight; k++)
			{
				Weight *currentWeight = &currentMesh->weights[currentVertex->startWeight + k];
				Joint *currentJoint = &this->joints[currentWeight->joint];

				XMVECTOR jointOrientation = XMVectorSet(
					currentJoint->orientation.x,
					currentJoint->orientation.y,
					currentJoint->orientation.z,
					currentJoint->orientation.w);

				XMVECTOR weightPosition = XMVectorSet(
					currentWeight->position.x,
					currentWeight->position.y,
					currentWeight->position.z,
					0);

				XMVECTOR jointConjugatedOrientation = XMVectorSet(
					-currentJoint->orientation.x,
					-currentJoint->orientation.y,
					-currentJoint->orientation.z,
					currentJoint->orientation.w);

				XMFLOAT3 rotatedVertex;
				XMStoreFloat3(&rotatedVertex, XMQuaternionMultiply(XMQuaternionMultiply(jointOrientation, weightPosition), jointConjugatedOrientation));

				currentVertex->position.x += ( currentJoint->position.x + rotatedVertex.x) * currentWeight->bias;
				currentVertex->position.y += ( currentJoint->position.y + rotatedVertex.y) * currentWeight->bias;
				currentVertex->position.z += ( currentJoint->position.z + rotatedVertex.z) * currentWeight->bias;
			}
		}
	}

	void ComputeNormals(Mesh *currentMesh)
	{
		XMVECTOR edge1 = XMVectorSet(0, 0, 0, 0);
		XMVECTOR edge2 = XMVectorSet(0, 0, 0, 0);
		float edgeX, edgeY, edgeZ;
		Vertex *vertex1, *vertex2, *vertex3;

		for (int i = 0; i < currentMesh->numTriangles; i++)
		{
			vertex1 = &currentMesh->vertices[currentMesh->triangles[i].vertexIndices[0]];
			vertex2 = &currentMesh->vertices[currentMesh->triangles[i].vertexIndices[1]];
			vertex3 = &currentMesh->vertices[currentMesh->triangles[i].vertexIndices[2]];

			edgeX = vertex1->position.x - vertex3->position.x;
			edgeY = vertex1->position.y - vertex3->position.y;
			edgeZ = vertex1->position.z - vertex3->position.z;
			edge1 = XMVectorSet(edgeX, edgeY, edgeZ, 0);

			edgeX = vertex2->position.x - vertex3->position.x;
			edgeY = vertex2->position.y - vertex3->position.y;
			edgeZ = vertex2->position.z - vertex3->position.z;
			edge2 = XMVectorSet(edgeX, edgeY, edgeZ, 0);

			XMFLOAT3 normal;
			XMStoreFloat3(&normal, XMVector3Cross(edge1, edge2));
			vertex1->normal = XMFLOAT3(vertex1->normal.x + normal.x, vertex1->normal.y + normal.y, vertex1->normal.z + normal.z);
			vertex2->normal = XMFLOAT3(vertex2->normal.x + normal.x, vertex2->normal.y + normal.y, vertex2->normal.z + normal.z);
			vertex3->normal = XMFLOAT3(vertex3->normal.x + normal.x, vertex3->normal.y + normal.y, vertex3->normal.z + normal.z);

			vertex1->timesUsed++;
			vertex2->timesUsed++;
			vertex3->timesUsed++;
		}

		for (int i = 0; i < currentMesh->numVertices; i++)
		{
			Vertex *currentVertex = &currentMesh->vertices[i];
			XMVECTOR normalSum = XMVectorSet(currentVertex->normal.x, currentVertex->normal.y, currentVertex->normal.z, 0);
			normalSum = XMVector3Normalize(normalSum / currentVertex->timesUsed);
			currentVertex->normal.x = XMVectorGetX(normalSum);
			currentVertex->normal.y = XMVectorGetY(normalSum);
			currentVertex->normal.z = XMVectorGetZ(normalSum);
		}
	}

	void ReadNumJointsAndMeshes(ifstream &fileStream)
	{
		string currentLine;
		while (getline(fileStream, currentLine))
		{
			string *currentLineSplitted = new string[2];
			SplitString(currentLine, 2, currentLineSplitted);

			if (currentLineSplitted[0] == "numJoints")
				this->numJoints = atoi(currentLineSplitted[1].c_str());
			else if (currentLineSplitted[0] == "numMeshes")
			{
				this->numMeshes = atoi(currentLineSplitted[1].c_str());
				return;
			}
		}
	}

	void ReadJoints(ifstream &fileStream)
	{
		string currentLine;
		bool isJointsZone = false;
		this->joints = new Joint[this->numJoints];
		int i = 0;

		while (getline(fileStream, currentLine) && i < this->numJoints)
		{
			if (isJointsZone)
			{
				string *currentLineSplitted = new string[12];			
				SplitString(currentLine, 12, currentLineSplitted);

				this->joints[i].name = currentLineSplitted[0];
				TrimString(this->joints[i].name, "\"");
				this->joints[i].parent = atoi(currentLineSplitted[1].c_str());
				this->joints[i].position.x = (float)atof(currentLineSplitted[3].c_str());
				this->joints[i].position.y = (float)atof(currentLineSplitted[4].c_str());
				this->joints[i].position.z = (float)atof(currentLineSplitted[5].c_str());
				this->joints[i].orientation.x = (float)atof(currentLineSplitted[8].c_str());
				this->joints[i].orientation.y = (float)atof(currentLineSplitted[9].c_str());
				this->joints[i].orientation.z = (float)atof(currentLineSplitted[10].c_str());
				this->joints[i].orientation.w = GetWComponent(this->joints[i].orientation);

				i++;
			}
			else
			{
				if (currentLine == "joints {")
					isJointsZone = true;
			}
		}
	}

	void ReadMeshes(ifstream &fileStream)
	{
		string currentLine;
		bool isMeshesZone = false;
		this->meshes = new Mesh[this->numMeshes];		
		int i = 0;

		while (getline(fileStream, currentLine) && i < this->numMeshes)
		{
			if (isMeshesZone)
			{
				if (currentLine == "}")
				{
					isMeshesZone = false;
					i++;
					continue;
				}

				string *currentLineSplitted = new string[2];
				SplitString(currentLine, 2, currentLineSplitted);

				if (currentLineSplitted[0] == "shader")
				{
					this->meshes[i].shader = currentLineSplitted[1];
					TrimString(this->meshes[i].shader, "\"");
				}
				else if (currentLineSplitted[0] == "numverts")
				{
					this->meshes[i].numVertices = atoi(currentLineSplitted[1].c_str());
					this->meshes[i].vertices = new Vertex[this->meshes[i].numVertices];

					for (int j = 0; j < this->meshes[i].numVertices; j++)
					{
						getline(fileStream, currentLine);
						string *vertLineSplitted = new string[8];
						SplitString(currentLine, 8, vertLineSplitted);

						this->meshes[i].vertices[j].vertexIndex = atoi(vertLineSplitted[1].c_str());
						this->meshes[i].vertices[j].uv.x = (float)atof(vertLineSplitted[3].c_str());
						this->meshes[i].vertices[j].uv.y = (float)atof(vertLineSplitted[4].c_str());
						this->meshes[i].vertices[j].startWeight = atoi(vertLineSplitted[6].c_str());
						this->meshes[i].vertices[j].countWeight = atoi(vertLineSplitted[7].c_str());
					}
				}
				else if (currentLineSplitted[0] == "numtris")
				{
					this->meshes[i].numTriangles = atoi(currentLineSplitted[1].c_str());
					this->meshes[i].triangles = new Triangle[this->meshes[i].numTriangles];
					this->meshes[i].indices	  = new int[this->meshes[i].numTriangles * 3];

					for (int j = 0; j < this->meshes[i].numTriangles; j++)
					{
						getline(fileStream, currentLine);
						string *triLineSplitted = new string[5];
						SplitString(currentLine, 5, triLineSplitted);
						
						this->meshes[i].triangles[j].triangleIndex = atoi(triLineSplitted[1].c_str());
						this->meshes[i].indices[j * 3] = this->meshes[i].triangles[j].vertexIndices[0] = atoi(triLineSplitted[2].c_str());
						this->meshes[i].indices[j * 3 + 1] = this->meshes[i].triangles[j].vertexIndices[1] = atoi(triLineSplitted[3].c_str());
						this->meshes[i].indices[j * 3 + 2] = this->meshes[i].triangles[j].vertexIndices[2] = atoi(triLineSplitted[4].c_str());
					}
				}
				else if (currentLineSplitted[0] == "numweights")
				{
					this->meshes[i].numWeights = atoi(currentLineSplitted[1].c_str());
					this->meshes[i].weights = new Weight[this->meshes[i].numWeights];

					for (int j = 0; j < this->meshes[i].numWeights; j++)
					{
						getline(fileStream, currentLine);
						string *weightLineSplitted = new string[9];
						SplitString(currentLine, 9, weightLineSplitted);

						this->meshes[i].weights[j].weightIndex = atoi(weightLineSplitted[1].c_str());
						this->meshes[i].weights[j].joint = atoi(weightLineSplitted[2].c_str());
						this->meshes[i].weights[j].bias = (float)atof(weightLineSplitted[3].c_str());
						this->meshes[i].weights[j].position.x = (float)atof(weightLineSplitted[5].c_str());
						this->meshes[i].weights[j].position.y = (float)atof(weightLineSplitted[6].c_str());
						this->meshes[i].weights[j].position.z = (float)atof(weightLineSplitted[7].c_str());
					}
				}
			}
			else
			{
				if (currentLine == "mesh {")
					isMeshesZone = true;
			}
		}
	}

#pragma endregion
};

#endif