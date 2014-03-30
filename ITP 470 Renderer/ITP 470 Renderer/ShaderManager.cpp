#include "ShaderManager.h"
#include "FileReaderWriter.h"
#include <algorithm>
#include <assert.h>

ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

std::string ShaderManager::AddPixelShader(const std::string &inShaderFilename)
{
	for (auto &it : mPixelShaders)
	{
		if (it.first.compare(inShaderFilename.c_str()) == 0)
		{
			return it.first;
		}
	}

	ShaderBinaryData *pixelShaderData;
	bool readSuccessful = FileReaderWriter::ReadShader(inShaderFilename.c_str(), pixelShaderData);

	if (!readSuccessful)
	{

	}

	md3dDevice->CreatePixelShader(pixelShaderData->shaderByteData, pixelShaderData->size, nullptr, &mPixelShaders[inShaderFilename]);

	delete pixelShaderData;

	return inShaderFilename;
}


std::string ShaderManager::AddVertexShader(const std::string &inShaderFilename, D3D11_INPUT_ELEMENT_DESC* inInputLayoutDesc, int inSizeOfLayoutDesc)
{
	for (auto &it : mVSWithLayouts)
	{
		if (it.first.compare(inShaderFilename.c_str()) == 0)
		{
			return it.first;
		}
	}

	ShaderBinaryData *vertexShaderData;
	bool readSuccessful = FileReaderWriter::ReadShader(inShaderFilename.c_str(), vertexShaderData);

	if (!readSuccessful)
	{

	}

	md3dDevice->CreateVertexShader(vertexShaderData->shaderByteData, vertexShaderData->size, nullptr, &mVSWithLayouts[inShaderFilename].mVertexShader);
	md3dDevice->CreateInputLayout(inInputLayoutDesc, inSizeOfLayoutDesc, vertexShaderData->shaderByteData, vertexShaderData->size, &mVSWithLayouts[inShaderFilename].mInputLayout);

	delete vertexShaderData;

	return inShaderFilename;
}

void ShaderManager::SetVertexShader(const std::string &inShaderName)
{
	if (mCurrentVertexShader.compare(inShaderName.c_str()) == 0)
	{
		return;
	}
#if DEBUG
	if (mVSWithLayouts.find(inShaderName) == mVSWithLayouts.end())
	{
		assert(false);
	}
#endif
	mCurrentVertexShader = inShaderName;
	md3dDeviceContext->VSSetShader(mVSWithLayouts[inShaderName].mVertexShader, 0, 0);
	md3dDeviceContext->IASetInputLayout(mVSWithLayouts[inShaderName].mInputLayout);
}

void ShaderManager::SetPixelShader(const std::string &inShaderName)
{
	if (mCurrentPixelShader.compare(inShaderName.c_str()) == 0)
	{
		return;
	}
#if DEBUG
	if (mPSWithLayouts.find(inShaderName) == mPSWithLayouts.end())
	{
		assert(false);
	}
#endif
	mCurrentPixelShader = inShaderName;
	md3dDeviceContext->PSSetShader(mPixelShaders[inShaderName], 0, 0);
}
