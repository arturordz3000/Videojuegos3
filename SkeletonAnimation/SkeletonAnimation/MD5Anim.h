#ifndef _ANIM_H_INCLUDED
#define _ANIM_H_INCLUDED

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

using namespace std;

class MD5Anim
{
	string filename;

	int numJoints;
	int numFrames;
	int frameRate;
	int numAnimatedComponents;

	float frameTime;
	float totalAnimationTime;
	float currentAnimationTime;

	vector<HierarchyInfo> hierarchy;
	vector<Bound> bounds;
	vector<BaseFrameInfo> baseFrame;
	vector<Frame> frames;

public:
	MD5Anim(string filename)
	{
		this->filename = filename;
		ifstream fileStream(filename + ".md5anim", ifstream::in);

		if (fileStream.is_open())
		{
			ReadGlobalParameters(fileStream);
			ReadHierarchy(fileStream);
			ReadBounds(fileStream);
			ReadBaseFrame(fileStream);
			ReadFrames(fileStream);

			ComputeTimes();
			ComputeFrameSkeletons();
		}
	}

public:
	void ComputeTimes()
	{
		frameTime = 1.0f / frameRate;
		totalAnimationTime = numFrames * frameTime;
		currentAnimationTime = 0;
	}

	void ComputeFrameSkeletons()
	{
		for (int i = 0; i < numFrames; i++)
		{
			for (int j = 0; j < numJoints; j++)
			{
				int k = 0;
				Joint currentJoint = BuildMeshJointWithAnimationInfo(&hierarchy[j], &baseFrame[j]);

				if (hierarchy[j].flags & 1)
					currentJoint.position.x = frames[i].parameters[hierarchy[j].startIndex + k++];
				if (hierarchy[j].flags & 2)
					currentJoint.position.z = frames[i].parameters[hierarchy[j].startIndex + k++];
				if (hierarchy[j].flags & 4)
					currentJoint.position.y = frames[i].parameters[hierarchy[j].startIndex + k++];
				if (hierarchy[j].flags & 8)
					currentJoint.orientation.x = frames[i].parameters[hierarchy[j].startIndex + k++];
				if (hierarchy[j].flags & 16)
					currentJoint.orientation.z = frames[i].parameters[hierarchy[j].startIndex + k++];
				if (hierarchy[j].flags & 32)
					currentJoint.orientation.y = frames[i].parameters[hierarchy[j].startIndex + k++];

				currentJoint.orientation.w = GetWComponent(currentJoint.orientation);

				if (hierarchy[j].parent >= 0)
				{
					Joint parentJoint = frames[i].skeleton[hierarchy[j].parent];
					XMVECTOR parentJointOrientation = XMVectorSet(parentJoint.orientation.x, 
																  parentJoint.orientation.y, 
																  parentJoint.orientation.z, 
																  parentJoint.orientation.w);
					XMVECTOR currentJointPosition = XMVectorSet(currentJoint.position.x,
																currentJoint.position.y,
																currentJoint.position.z,
																0);
					XMVECTOR parentJointConjugatedOrientation = XMVectorSet(-parentJoint.orientation.x, 
																  -parentJoint.orientation.y, 
																  -parentJoint.orientation.z, 
																  parentJoint.orientation.w);

					XMFLOAT3 rotatedPosition;
					XMStoreFloat3(&rotatedPosition, XMQuaternionMultiply(XMQuaternionMultiply(parentJointOrientation, currentJointPosition), parentJointConjugatedOrientation));

					currentJoint.position.x = rotatedPosition.x + parentJoint.position.x;
					currentJoint.position.y = rotatedPosition.y + parentJoint.position.y;
					currentJoint.position.z = rotatedPosition.z + parentJoint.position.z;

					XMVECTOR currentJointOrientation = XMVectorSet(currentJoint.orientation.x, 
																   currentJoint.orientation.y,
																   currentJoint.orientation.z, 
																   currentJoint.orientation.w);
					currentJointOrientation = XMQuaternionMultiply(parentJointOrientation, currentJointOrientation);
					currentJointOrientation = XMQuaternionNormalize(currentJointOrientation);
					XMStoreFloat4(&currentJoint.orientation, currentJointOrientation);
				}
				
				frames[i].skeleton.push_back(currentJoint);
			}
		}
	}

	void UpdateModel(vector<Mesh>& meshes, float deltaTime, ID3D11DeviceContext *deviceContext)
	{
		currentAnimationTime += deltaTime;
		if (currentAnimationTime > totalAnimationTime)
			currentAnimationTime = 0;

		float currentFrame = currentAnimationTime * frameRate;
		int frame0 = floorf(currentFrame);
		int frame1 = frame0 == numFrames - 1 ? 0 : frame0 + 1;

		float interpolation = currentFrame - frame0;

		vector<Joint> interpolatedSkeleton;

		for (int i = 0; i < numJoints; i++)
		{
			Joint currentJoint;
			Joint joint0 = frames[frame0].skeleton[i];
			Joint joint1 = frames[frame1].skeleton[i];

			currentJoint.parent = joint0.parent;

			XMVECTOR joint0Orientation = XMVectorSet(joint0.orientation.x, joint0.orientation.y, joint0.orientation.z, joint0.orientation.w);
			XMVECTOR joint1Orientation = XMVectorSet(joint1.orientation.x, joint1.orientation.y, joint1.orientation.z, joint1.orientation.w);

			currentJoint.position.x = joint1.position.x * interpolation + (1 - interpolation) * joint0.position.x;
			currentJoint.position.y = joint1.position.y * interpolation + (1 - interpolation) * joint0.position.y;
			currentJoint.position.z = joint1.position.z * interpolation + (1 - interpolation) * joint0.position.z;

			XMStoreFloat4(&currentJoint.orientation, XMQuaternionSlerp(joint0Orientation, joint1Orientation, interpolation));
			interpolatedSkeleton.push_back(currentJoint);
		}

		for (int i = 0; i < meshes.size(); i++)
		{
			for (int j = 0; j < meshes[i].numVertices; j++)
			{
				Vertex currentVertex = meshes[i].vertices[j];
				currentVertex.position = XMFLOAT3(0, 0, 0);
				currentVertex.normal = XMFLOAT3(0, 0, 0);

				for (int k = 0; k < currentVertex.countWeight; k++)
				{
					Weight currentWeight = meshes[i].weights[currentVertex.startWeight + k];
					Joint interpolatedJoint = interpolatedSkeleton[currentWeight.joint];

					XMVECTOR interpolatedJointOrientation = XMVectorSet(interpolatedJoint.orientation.x, 
																		interpolatedJoint.orientation.y, 
																		interpolatedJoint.orientation.z, 
																		interpolatedJoint.orientation.w);
					XMVECTOR currentWeightPosition = XMVectorSet(currentWeight.position.x,
																 currentWeight.position.y,
																 currentWeight.position.z,
																 0);
					XMVECTOR interpolatedJointConjugatedOrientation = XMVectorSet(-interpolatedJoint.orientation.x, 
																		-interpolatedJoint.orientation.y, 
																		-interpolatedJoint.orientation.z, 
																		interpolatedJoint.orientation.w);

					XMFLOAT3 rotatedPoint;
					XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(interpolatedJointOrientation, currentWeightPosition),
						interpolatedJointConjugatedOrientation));

					currentVertex.position.x += (interpolatedJoint.position.x + rotatedPoint.x) * currentWeight.bias;
					currentVertex.position.y += (interpolatedJoint.position.y + rotatedPoint.y) * currentWeight.bias;
					currentVertex.position.z += (interpolatedJoint.position.z + rotatedPoint.z) * currentWeight.bias;

					XMVECTOR tempWeightNormal = XMVectorSet(currentWeight.normal.x, currentWeight.normal.y, currentWeight.normal.z, 0.0f);

					// Rotate the normal
					XMStoreFloat3(&rotatedPoint, XMQuaternionMultiply(XMQuaternionMultiply(interpolatedJointOrientation, tempWeightNormal), interpolatedJointConjugatedOrientation));

					// Add to vertices normal and ake weight bias into account
					currentVertex.normal.x -= rotatedPoint.x * currentWeight.bias;
					currentVertex.normal.y -= rotatedPoint.y * currentWeight.bias;
					currentVertex.normal.z -= rotatedPoint.z * currentWeight.bias;
				}

				meshes[i].vertices[j] = currentVertex;
			}

			D3D11_MAPPED_SUBRESOURCE mappedVertexBuffer;
			HRESULT hResult = deviceContext->Map(meshes[i].vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertexBuffer);
			memcpy(mappedVertexBuffer.pData, &meshes[i].vertices[0], (sizeof(Vertex) * meshes[i].vertices.size()));
			deviceContext->Unmap(meshes[i].vertexBuffer, 0);
		}
	}

	void ReadGlobalParameters(ifstream &fileStream)
	{
		string currentLine;
		while (getline(fileStream, currentLine))
		{
			string *currentLineSplitted = new string[2];
			SplitString(currentLine, 2, currentLineSplitted);

			if (currentLineSplitted[0] == "numFrames")
				this->numFrames = atoi(currentLineSplitted[1].c_str());
			else if (currentLineSplitted[0] == "numJoints")
				this->numJoints = atoi(currentLineSplitted[1].c_str());
			else if (currentLineSplitted[0] == "frameRate")
				this->frameRate = atoi(currentLineSplitted[1].c_str());
			else if (currentLineSplitted[0] == "numAnimatedComponents")
			{
				this->numAnimatedComponents = atoi(currentLineSplitted[1].c_str());
				delete[] currentLineSplitted;
				return;
			}

			delete[] currentLineSplitted;
		}
	}

	void ReadHierarchy(ifstream &fileStream)
	{
		string currentLine;

		while (getline(fileStream, currentLine))
			if (currentLine == "hierarchy {") break;

		int i = 0;

		while (getline(fileStream, currentLine) && i < this->numJoints)
		{
			HierarchyInfo hierarchyInfo;
			string *currentLineSplitted = new string[4];
			SplitString(currentLine, 4, currentLineSplitted);
			
			hierarchyInfo.name = currentLineSplitted[0];
			TrimString(hierarchyInfo.name, "\"");
			hierarchyInfo.parent = atoi(currentLineSplitted[1].c_str());
			hierarchyInfo.flags = atoi(currentLineSplitted[2].c_str());
			hierarchyInfo.startIndex = atoi(currentLineSplitted[3].c_str());

			hierarchy.push_back(hierarchyInfo);
			delete[] currentLineSplitted;

			i++;
		}
	}
	 
	void ReadBounds(ifstream &fileStream)
	{
		string currentLine;

		while (getline(fileStream, currentLine))
			if (currentLine == "bounds {") break;

		while (getline(fileStream, currentLine))
		{
			if (currentLine == "}") break;

			Bound bound;
			string *currentLineSplitted = new string[10];
			SplitString(currentLine, 10, currentLineSplitted);
			
			bound.min.x = atof(currentLineSplitted[1].c_str());
			bound.min.z = atof(currentLineSplitted[2].c_str());
			bound.min.y = atof(currentLineSplitted[3].c_str());

			bound.max.x = atof(currentLineSplitted[6].c_str());
			bound.max.z = atof(currentLineSplitted[7].c_str());
			bound.max.y = atof(currentLineSplitted[8].c_str());

			bounds.push_back(bound);
			delete[] currentLineSplitted;
		}
	}

	void ReadBaseFrame(ifstream &fileStream)
	{
		string currentLine;

		while (getline(fileStream, currentLine))
			if (currentLine == "baseframe {") break;

		while (getline(fileStream, currentLine))
		{
			if (currentLine == "}") break;

			BaseFrameInfo baseFrameInfo;
			string *currentLineSplitted = new string[10];
			SplitString(currentLine, 10, currentLineSplitted);
			
			baseFrameInfo.position.x = atof(currentLineSplitted[1].c_str());
			baseFrameInfo.position.z = atof(currentLineSplitted[2].c_str());
			baseFrameInfo.position.y = atof(currentLineSplitted[3].c_str());

			baseFrameInfo.orientation.x = atof(currentLineSplitted[6].c_str());
			baseFrameInfo.orientation.z = atof(currentLineSplitted[7].c_str());
			baseFrameInfo.orientation.y = atof(currentLineSplitted[8].c_str());
			baseFrameInfo.orientation.w = GetWComponent(baseFrameInfo.orientation);

			baseFrame.push_back(baseFrameInfo);
			delete[] currentLineSplitted;
		}
	}

	void ReadFrames(ifstream &fileStream)
	{
		string currentLine;

		while (getline(fileStream, currentLine))
		{
			if (currentLine == "") continue;

			string *currentLineSplitted = new string[3];
			SplitString(currentLine, 3, currentLineSplitted);

			Frame frame;
			frame.frameIndex = atoi(currentLineSplitted[1].c_str());

			delete[] currentLineSplitted;
				
			while (getline(fileStream, currentLine))
			{
				if (currentLine == "}") break;

				float parameters[6];
				currentLineSplitted = new string[6];
				SplitString(currentLine, 6, currentLineSplitted);
			
				frame.parameters.push_back(atof(currentLineSplitted[0].c_str()));
				frame.parameters.push_back(atof(currentLineSplitted[1].c_str()));
				frame.parameters.push_back(atof(currentLineSplitted[2].c_str()));
				frame.parameters.push_back(atof(currentLineSplitted[3].c_str()));
				frame.parameters.push_back(atof(currentLineSplitted[4].c_str()));
				frame.parameters.push_back(atof(currentLineSplitted[5].c_str()));			
			}

			frames.push_back(frame);
		}
	}
};

#endif