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
	XMFLOAT2 uv;
	int startWeight;
	int countWeight;
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
				this->joints[i].position.x = atof(currentLineSplitted[3].c_str());
				this->joints[i].position.y = atof(currentLineSplitted[4].c_str());
				this->joints[i].position.z = atof(currentLineSplitted[5].c_str());
				this->joints[i].orientation.x = atof(currentLineSplitted[8].c_str());
				this->joints[i].orientation.y = atof(currentLineSplitted[9].c_str());
				this->joints[i].orientation.z = atof(currentLineSplitted[10].c_str());
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
						this->meshes[i].vertices[j].uv.x = atof(vertLineSplitted[3].c_str());
						this->meshes[i].vertices[j].uv.y = atof(vertLineSplitted[4].c_str());
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
						this->meshes[i].weights[j].bias = atof(weightLineSplitted[3].c_str());
						this->meshes[i].weights[j].position.x = atof(weightLineSplitted[5].c_str());
						this->meshes[i].weights[j].position.y = atof(weightLineSplitted[6].c_str());
						this->meshes[i].weights[j].position.z = atof(weightLineSplitted[7].c_str());
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