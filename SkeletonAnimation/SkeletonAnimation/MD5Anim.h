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

using namespace std;

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
};

class MD5Anim
{
	string filename;

	int numJoints;
	int numFrames;
	int frameRate;
	int numAnimatedComponents;

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
		}
	}

private:
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
			bound.min.y = atof(currentLineSplitted[2].c_str());
			bound.min.z = atof(currentLineSplitted[3].c_str());

			bound.max.x = atof(currentLineSplitted[6].c_str());
			bound.max.y = atof(currentLineSplitted[7].c_str());
			bound.max.z = atof(currentLineSplitted[8].c_str());

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
			baseFrameInfo.position.y = atof(currentLineSplitted[2].c_str());
			baseFrameInfo.position.z = atof(currentLineSplitted[3].c_str());

			baseFrameInfo.orientation.x = atof(currentLineSplitted[6].c_str());
			baseFrameInfo.orientation.y = atof(currentLineSplitted[7].c_str());
			baseFrameInfo.orientation.z = atof(currentLineSplitted[8].c_str());
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