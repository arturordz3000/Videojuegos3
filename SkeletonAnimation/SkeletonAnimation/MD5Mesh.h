#ifndef _MESH_H_INCLUDED
#define _MESH_H_INCLUDED

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
	vector<Vertex> vertices;
	vector<Triangle> triangles;
	vector<Weight> weights;
	vector<int> indices;

	~Mesh()
	{
		vertices.clear();
		triangles.clear();
		weights.clear();
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
	vector<Joint> joints;
	vector<Mesh> meshes;

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
		joints.clear();
		meshes.clear();
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
		//this->joints = new Joint[this->numJoints];
		int i = 0;

		while (getline(fileStream, currentLine) && i < this->numJoints)
		{
			if (isJointsZone)
			{
				Joint joint;
				string *currentLineSplitted = new string[12];			
				SplitString(currentLine, 12, currentLineSplitted);

				joint.name = currentLineSplitted[0];
				TrimString(joint.name, "\"");
				joint.parent = atoi(currentLineSplitted[1].c_str());
				joint.position.x = (float)atof(currentLineSplitted[3].c_str());
				joint.position.z = (float)atof(currentLineSplitted[4].c_str());
				joint.position.y = (float)atof(currentLineSplitted[5].c_str());
				joint.orientation.x = (float)atof(currentLineSplitted[8].c_str());
				joint.orientation.z = (float)atof(currentLineSplitted[9].c_str());
				joint.orientation.y = (float)atof(currentLineSplitted[10].c_str());
				joint.orientation.w = GetWComponent(joint.orientation);

				joints.push_back(joint);

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
		//this->meshes = new Mesh[this->numMeshes];		
		int i = 0;
		
		Mesh *mesh = new Mesh();

		while (getline(fileStream, currentLine) && i < this->numMeshes)
		{
			if (isMeshesZone)
			{
				if (currentLine == "}")
				{
					this->meshes.push_back(*mesh);
					mesh = new Mesh();
					isMeshesZone = false;
					i++;
					continue;
				}

				string *currentLineSplitted = new string[2];
				SplitString(currentLine, 2, currentLineSplitted);

				if (currentLineSplitted[0] == "shader")
				{
					mesh->shader = currentLineSplitted[1];
					TrimString(mesh->shader, "\"");
				}
				else if (currentLineSplitted[0] == "numverts")
				{
					mesh->numVertices = atoi(currentLineSplitted[1].c_str());
					//mesh->vertices = new Vertex[this->meshes[i].numVertices];

					for (int j = 0; j < mesh->numVertices; j++)
					{
						Vertex vertex;
						getline(fileStream, currentLine);
						string *vertLineSplitted = new string[8];
						SplitString(currentLine, 8, vertLineSplitted);

						vertex.vertexIndex = atoi(vertLineSplitted[1].c_str());
						vertex.uv.x = (float)atof(vertLineSplitted[3].c_str());
						vertex.uv.y = (float)atof(vertLineSplitted[4].c_str());
						vertex.startWeight = atoi(vertLineSplitted[6].c_str());
						vertex.countWeight = atoi(vertLineSplitted[7].c_str());

						mesh->vertices.push_back(vertex);
					}
				}
				else if (currentLineSplitted[0] == "numtris")
				{
					mesh->numTriangles = atoi(currentLineSplitted[1].c_str());
					//mesh->triangles = new Triangle[this->meshes[i].numTriangles];
					//mesh->indices	  = new int[this->meshes[i].numTriangles * 3];

					for (int j = 0; j < mesh->numTriangles; j++)
					{
						Triangle triangle;
						int indices[3];

						getline(fileStream, currentLine);
						string *triLineSplitted = new string[5];
						SplitString(currentLine, 5, triLineSplitted);
						
						triangle.triangleIndex = atoi(triLineSplitted[1].c_str());
						indices[0] = triangle.vertexIndices[0] = atoi(triLineSplitted[2].c_str());
						indices[1] = triangle.vertexIndices[1] = atoi(triLineSplitted[3].c_str());
						indices[2] = triangle.vertexIndices[2] = atoi(triLineSplitted[4].c_str());

						mesh->triangles.push_back(triangle);
						mesh->indices.push_back(indices[0]);
						mesh->indices.push_back(indices[1]);
						mesh->indices.push_back(indices[2]);
					}
				}
				else if (currentLineSplitted[0] == "numweights")
				{
					mesh->numWeights = atoi(currentLineSplitted[1].c_str());
					//this->meshes[i].weights = new Weight[this->meshes[i].numWeights];

					for (int j = 0; j < mesh->numWeights; j++)
					{
						Weight weight;
						getline(fileStream, currentLine);
						string *weightLineSplitted = new string[9];
						SplitString(currentLine, 9, weightLineSplitted);

						weight.weightIndex = atoi(weightLineSplitted[1].c_str());
						weight.joint = atoi(weightLineSplitted[2].c_str());
						weight.bias = (float)atof(weightLineSplitted[3].c_str());
						weight.position.x = (float)atof(weightLineSplitted[5].c_str());
						weight.position.z = (float)atof(weightLineSplitted[6].c_str());
						weight.position.y = (float)atof(weightLineSplitted[7].c_str());

						mesh->weights.push_back(weight);
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