#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <tuple>
#include "assimp/include/mesh.h"
#include "assimp/include/scene.h"
#include "MeshData.h"



using namespace DirectX;

struct Material
{
	XMFLOAT4 mAmbient;
	XMFLOAT4 mDiffuse;
	XMFLOAT3 mSpecular; float mShininess; //w component of specular is shininess
};

struct perObjectCBVSStruct
{
	XMFLOAT4X4 mWorld;
};
struct perObjectCBPSStruct
{
	Material mMaterial;
};

class DrawableObject
{
public:
	DrawableObject();

	~DrawableObject();
	void Draw(ID3D11DeviceContext* d3dDeviceContext);
	void Update(float deltaTime);

	MeshData* GetMeshData()								{ return mMeshData; }

	void SetVSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectVSCB = inBuffer; }
	ID3D11Buffer* GetVSConstantBuffer()					{ return perObjectVSCB; }

	void SetPSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectPSCB = inBuffer; }
	ID3D11Buffer* GetPSConstantBuffer()					{ return perObjectPSCB; }

	void SetPixelShader(const std::string &inPixelShader) { pixelShaderID = inPixelShader; }
	void SetVertexShader(const std::string &inVertexShader) { vertexShaderID = inVertexShader; }
	std::string GetPixelShader() { return pixelShaderID; }
	std::string GetVertexShader() { return vertexShaderID; }

	void SetSamplerState(ID3D11SamplerState *inSamplerState)	{ textureSampler = inSamplerState; }
	ID3D11SamplerState *GetSamplerState()				{ return textureSampler; }

	void SetMaterial(const aiColor3D &inAmbient, const aiColor3D &inDiffuse, const aiColor3D &inSpecular, float inShininess)
	{
		mMaterial.mAmbient = XMFLOAT4(inAmbient.r, inAmbient.g, inAmbient.b, 1.0f);
		mMaterial.mDiffuse = XMFLOAT4(inDiffuse.r, inDiffuse.g, inDiffuse.b, 1.0f);
		mMaterial.mSpecular = XMFLOAT3(inSpecular.r, inSpecular.g, inSpecular.b);
		mMaterial.mShininess = inShininess;
	}

	void SetPosition(const XMFLOAT3 &inPosition) { mPosition = inPosition; }

	void SetRotation(const XMFLOAT4 &inRotation) { mRotation = inRotation; }

	void SetScale(float inScale) { mScale = inScale; }

	void LoadWorldTransform(XMFLOAT4X4* inMatrix)
	{
		XMStoreFloat4x4(
			inMatrix, 
			XMMatrixScaling(mScale, mScale, mScale) *
			XMMatrixRotationQuaternion(XMLoadFloat4(&mRotation)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&mPosition))
			);
	}

	void AddPart(UINT inVertexBufferStart, UINT inIndexBufferStart, int inNumIndices, unsigned int inMaterialIndex)
	{
		//mParts.push_back(std::make_tuple(inVertexBufferStart, inIndexBufferStart, inNumIndices, inMaterialIndex));
		mParts.push_back(Part(inVertexBufferStart, inIndexBufferStart, inNumIndices, inMaterialIndex));
	}

	void AddTexture(ID3D11ShaderResourceView* inDiffuseResourceView, ID3D11ShaderResourceView* inNormalResourceView)
	{
		mTextures.push_back(std::make_tuple(inDiffuseResourceView, inNormalResourceView));
	}

private:

	struct Part
	{
		Part(UINT inVertexBufferStart, UINT inIndexBufferStart, int inNumIndices, unsigned int inMaterialIndex) :
		mVertexBufferStart(inVertexBufferStart),
		mIndexBufferStart(inIndexBufferStart),
		mNumIndices(inNumIndices),
		mMaterialIndex(inMaterialIndex)
		{};

		UINT mVertexBufferStart;
		UINT mIndexBufferStart;
		int mNumIndices;
		unsigned int mMaterialIndex;
	};

	void BuildShaders(ID3D11Device* d3dDevice);
	void LoadTexture(ID3D11Device* d3dDevice, const aiScene* scene);


	Material	mMaterial;
	XMFLOAT4	mRotation;
	XMFLOAT3	mPosition;
	float		mRotationAmount;
	float		mScale;

	std::vector<Part> mParts;
	//std::vector<std::tuple<UINT, UINT, int, unsigned int> > mParts;
	std::vector<std::tuple<ID3D11ShaderResourceView*, ID3D11ShaderResourceView*> > mTextures; //diffuse + normal texture

	MeshData *mMeshData;

	ID3D11Buffer				*perObjectVSCB;
	ID3D11Buffer				*perObjectPSCB;
	ID3D11SamplerState			*textureSampler;
	std::string vertexShaderID, pixelShaderID;
};
