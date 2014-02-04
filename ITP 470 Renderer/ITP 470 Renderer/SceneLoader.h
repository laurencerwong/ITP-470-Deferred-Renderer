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

struct ObjectInitializeData
{
	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11Buffer				*perObjectConstantBuffer;
	ID3D11InputLayout			*inputLayout;
	ID3D11VertexShader			*vertexShader;
	ID3D11PixelShader			*pixelShader;
	ID3D11SamplerState			*textureSampler;
	ID3D11ShaderResourceView	*texture0View;
	UINT						mVertexBufferStride;
	int							numIndices;
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

