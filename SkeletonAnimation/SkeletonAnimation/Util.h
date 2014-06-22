#ifndef _UTIL_H_INCLUDED
#define _UTIL_H_INCLUDED

#define _XM_NO_INTRINSICS_

#include <string>
#include <sstream>
#include <math.h>
#include <xnamath.h>
#include <D3Dcompiler.h>

using namespace std;

void SplitString(string inputString, int parts, string *output)
{
	stringstream inputStringStream(inputString);

	int i = 0;
	while (inputStringStream.good() && i < parts)
	{
		inputStringStream >> output[i];
		i++;
	}
}

float GetWComponent(XMFLOAT4 q)
{
	float w = 1.0f - (q.x * q.x) - (q.y * q.y) - (q.z * q.z);
	return w < 0 ? 0 : -sqrt(w);
}

void TrimString(string &input, string trimChar)
{
	int startIndex = 0;
	
	while (true)
	{
		startIndex = input.find(trimChar);
		if (startIndex == string::npos)
			break;

		input.replace(startIndex, startIndex + trimChar.length(), "");
	}
}

void GetMonitorResolution(int *width, int *height)
{
	RECT windowsize;    // get the height and width of the screen
	int succeded = GetClientRect(GetDesktopWindow(), &windowsize);
	
	if ( succeded )
	{
		*width = windowsize.right - windowsize.left;
		*height = windowsize.bottom - windowsize.top;
	}
}

bool CompileD3DShader(LPCSTR filePath, char* entry, char* shaderModel, ID3DBlob** buffer)
{
	DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ID3DBlob* errorBuffer = 0;
	HRESULT result;

	result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags,
		0, 0, buffer, &errorBuffer, 0);
	if(FAILED(result))
	{
		if(errorBuffer != 0)
		{
			OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
			errorBuffer->Release();
		}
		return false;
	}

	if(errorBuffer != 0)
		errorBuffer->Release();

	return true;
}

#endif