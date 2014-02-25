#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <tuple>
#include "assimp/include/mesh.h"
#include "assimp/include/scene.h"
#include "ShaderManager.h"
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
	DrawableObject(ShaderManager *inShaderManager);

	~DrawableObject();
	void Draw(ID3D11DeviceContext* d3dDeviceContext);
	void Update(float deltaTime);
	void LoadFromString(std::string const& file, ID3D11Device* d3dDevice);

	MeshData* GetMeshData()								{ return mMeshData; }

	void SetVSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectVSCB = inBuffer; }
	ID3D11Buffer* GetVSConstantBuffer()					{ return perObjectVSCB; }

	void SetPSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectPSCB = inBuffer; }
	ID3D11Buffer* GetPSConstantBuffer()					{ return perObjectPSCB; }

	void SetPixelShader(const std::string &inPixelShader) { pixelShaderID = inPixelShader; }
	void SetVertexShader(const std::string &inVertexShader) { vertexShaderID = inVertexShader; }

	void SetSamplerState(ID3D11SamplerState *inSamplerState)	{ textureSampler = inSamplerState; }
	ID3D11SamplerState *GetSamplerState()				{ return textureSampler; }

	void SetDiffuseResourceView(ID3D11ShaderResourceView *inShaderResourceView)	{ texture0View = inShaderResourceView; }
	ID3D11ShaderResourceView *GetDiffuseResourceView()	{ return texture0View; }

	void SetNormalResourceView(ID3D11ShaderResourceView *inShaderResourceView)	{ textureNormView = inShaderResourceView; }
	ID3D11ShaderResourceView *GetNormalResourceView()	{ return textureNormView; }

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

	void AddPart(UINT inVertexBufferStart, UINT inIndexBufferStart, int inNumIndices, unsigned int inMaterialIndex)
	{
		mParts.push_back(std::make_tuple(inVertexBufferStart, inIndexBufferStart, inNumIndices, inMaterialIndex));
	}

	void AddTexture(ID3D11ShaderResourceView* inDiffuseResourceView, ID3D11ShaderResourceView* inNormalResourceView)
	{
		mTextures.push_back(std::make_tuple(inDiffuseResourceView, inNormalResourceView));
	}

private:
	void BuildShaders(ID3D11Device* d3dDevice);
	void LoadTexture(ID3D11Device* d3dDevice, const aiScene* scene);


	Material	mMaterial;
	XMFLOAT4	mRotation;
	XMFLOAT3	mPosition;
	float		mRotationAmount;
	float		mScale;

	//std::vector<std::tuple<ID3D11Buffer*, ID3D11Buffer*, int> > mParts;
	std::vector<std::tuple<UINT, UINT, int, unsigned int> > mParts;
	std::vector<std::tuple<ID3D11ShaderResourceView*, ID3D11ShaderResourceView*> > mTextures; //diffuse + normal texture

	ShaderManager *mShaderManager;

	MeshData *mMeshData;

	ID3D11Buffer				*perObjectVSCB;
	ID3D11Buffer				*perObjectPSCB;
	ID3D11SamplerState			*textureSampler;
	ID3D11ShaderResourceView	*texture0View;
	ID3D11ShaderResourceView	*textureNormView;
	std::string vertexShaderID, pixelShaderID;
};
