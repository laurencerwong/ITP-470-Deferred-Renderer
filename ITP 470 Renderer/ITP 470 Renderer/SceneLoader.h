#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "assimp/include/vector3.h"
#include <vector>
#include "ShaderManager.h"

using namespace DirectX;

class DrawableObject;

class SceneLoader
{
public:
	SceneLoader();
	SceneLoader(ID3D11Device *ind3dDevice, ShaderManager *inShaderManager) : d3dDevice(ind3dDevice), mShaderManager(inShaderManager) { };
	~SceneLoader();

	int LoadFile(const char* filename);
	std::vector<DrawableObject*> &GetDrawableObjects() { return mDrawableObjects; }
	DrawableObject* GetDrawableObject(int inDrawableObjectID) { return mDrawableObjects[inDrawableObjectID]; }
private:
	ID3D11Device *d3dDevice;
	ShaderManager *mShaderManager;
	std::vector<DrawableObject*>mDrawableObjects;
};

