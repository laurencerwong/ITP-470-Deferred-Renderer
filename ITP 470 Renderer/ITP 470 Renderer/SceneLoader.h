#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "assimp/include/vector3.h"
#include <vector>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Bitangent;
	XMFLOAT2 Tex0;

	inline void LoadAiVector3D(XMFLOAT3& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT3(inRHSVector.x, inRHSVector.y, inRHSVector.z);
	}
	inline void LoadAiVector3D(XMFLOAT2& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT2(inRHSVector.x, inRHSVector.y);
	}
};

class DrawableObject;

class SceneLoader
{
public:
	SceneLoader();
	SceneLoader(ID3D11Device *ind3dDevice) : d3dDevice(ind3dDevice) { };
	~SceneLoader();

	bool LoadFile(const char* filename);
	std::vector<DrawableObject*> &GetDrawableObjects() { return mDrawableObjects; }
private:
	ID3D11Device *d3dDevice;
	std::vector<DrawableObject*>mDrawableObjects;
};

