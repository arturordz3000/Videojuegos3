#ifndef _STRUCTS_H_INCLUDED
#define _STRUCTS_H_INCLUDED

#include <string>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

using namespace std;

#pragma region Mesh Substructures

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
	XMFLOAT3 normal;
};

struct Mesh
{
	string shader;
	int numVertices;
	int numTriangles;
	int numWeights;
	vector<Vertex> vertices;
	vector<Triangle> triangles;
	vector<Weight> weights;
	vector<int> indices;

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11ShaderResourceView *colorMap;

	~Mesh()
	{
		vertices.clear();
		triangles.clear();
		weights.clear();
	}
};

struct MatrixBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

struct ParticleBuffer
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
	float uvOffset;
	XMFLOAT3 padding;
};

#pragma endregion

#pragma region Animation Substructures

struct HierarchyInfo
{
	string name;
	int parent;
	int flags;
	int startIndex;
};

struct Bound
{
	XMFLOAT3 min;
	XMFLOAT3 max;
};

struct BaseFrameInfo
{
	XMFLOAT3 position;
	XMFLOAT4 orientation;
};

struct Frame
{
	int frameIndex;
	vector<float> parameters;
	vector<Joint> skeleton;
};


#pragma endregion


#pragma region Particle Substructures

struct ParticleComponent
{
	XMFLOAT3 pos;
	XMFLOAT4 color;
	XMFLOAT2 uv;
};

#pragma endregion

#endif