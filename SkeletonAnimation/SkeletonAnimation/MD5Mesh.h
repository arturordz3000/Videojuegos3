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
	int vertexIndex;
	int startWeight;
	int countWeight;

	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
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

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;

	~Mesh()
	{
		delete[] vertices;
		delete[] triangles;
		delete[] weights;
	}
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
		compileResult = CompileD3DShader("TestShader.fx", "VS_Main", "vs_4_0", &vertexShaderBlob);
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
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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
		compileResult = CompileD3DShader("TestShader.fx", "PS_Main", "vs_4_0", &pixelShaderBlob);
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

	bool CreateVertexAndIndexBuffers(ID3D11Device *device)
	{
		for (int i = 0; i < numMeshes; i++)
		{
			Mesh *currentMesh = &meshes[i];

			for (int j = 0; j < currentMesh->numVertices; j++)
			{
				Vertex *currentVertex = &currentMesh->vertices[j];
				currentVertex->position = XMFLOAT3(0, 0, 0);

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
				}
			}
		}

		return true;
	}

	bool PrepareGraphicResources(ID3D11Device *device)
	{
		if ( !CompileShaders(device) )
			return false;
		if ( !CreateVertexAndIndexBuffers(device) )
			return false;
	}

	void Update(float deltaTime)
	{

	}

	void Draw(ID3D11DeviceContext *deviceContext)
	{

	}

#pragma endregion

#pragma region Private methods

private:
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

					for (int j = 0; j < this->meshes[i].numTriangles; j++)
					{
						getline(fileStream, currentLine);
						string *triLineSplitted = new string[5];
						SplitString(currentLine, 5, triLineSplitted);
						
						this->meshes[i].triangles[j].triangleIndex = atoi(triLineSplitted[1].c_str());
						this->meshes[i].triangles[j].vertexIndices[0] = atoi(triLineSplitted[2].c_str());
						this->meshes[i].triangles[j].vertexIndices[1] = atoi(triLineSplitted[3].c_str());
						this->meshes[i].triangles[j].vertexIndices[2] = atoi(triLineSplitted[4].c_str());
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