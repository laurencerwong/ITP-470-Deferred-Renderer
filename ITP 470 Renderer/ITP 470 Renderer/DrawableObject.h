#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "assimp/include/mesh.h"
#include "assimp/include/scene.h"



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
	void LoadFromString(std::string const& file, ID3D11Device* d3dDevice);

	void SetVertexBuffer(ID3D11Buffer *inBuffer)		{ vertexBuffer = inBuffer; }
	ID3D11Buffer* GetVertexBuffer()						{ return vertexBuffer; }

	void SetIndexBuffer(ID3D11Buffer *inBuffer)			{ indexBuffer = inBuffer; }
	ID3D11Buffer* GetIndexBuffer()						{ return indexBuffer; }

	void SetNumIndicies(int inNumIndices)				{ numIndices = inNumIndices; }
	int	GetNumIndices()									{ return numIndices; }

	void SetVSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectVSCB = inBuffer; }
	ID3D11Buffer* GetVSConstantBuffer()					{ return perObjectVSCB; }

	void SetPSConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectPSCB = inBuffer; }
	ID3D11Buffer* GetPSConstantBuffer()					{ return perObjectPSCB; }

	void SetInputLayout(ID3D11InputLayout *inLayout)	{ inputLayout = inLayout; }
	ID3D11InputLayout* GetInputLayout()					{ return inputLayout; }

	void SetVertexBufferStride(UINT inStride)			{ mVertexBufferStride = inStride; }
	UINT GetVertexBufferStride()						{ return mVertexBufferStride; }

	void SetVertexShader(ID3D11VertexShader *inVertexShader)	{ vertexShader = inVertexShader; }
	ID3D11VertexShader *GetVertexShader()				{ return vertexShader; }

	void SetPixelShader(ID3D11PixelShader *inPixelShader)	{ pixelShader = inPixelShader; }
	ID3D11PixelShader *GetPixelShader()					{ return pixelShader; }

	void SetSamplerState(ID3D11SamplerState *inSamplerState)	{ textureSampler = inSamplerState; }
	ID3D11SamplerState *GetSamplerState()				{ return textureSampler; }

	void SetShaderResourceView(ID3D11ShaderResourceView *inShaderResourceView)	{ texture0View = inShaderResourceView; }
	ID3D11ShaderResourceView *GetShaderResourceView()	{ return texture0View; }

	void SetMaterial(const aiColor3D &inAmbient, const aiColor3D &inDiffuse, const aiColor3D &inSpecular, float inShininess)
	{
		mMaterial.mAmbient = XMFLOAT4(inAmbient.r, inAmbient.g, inAmbient.b, 1.0f);
		mMaterial.mDiffuse = XMFLOAT4(inDiffuse.r, inDiffuse.g, inDiffuse.b, 1.0f);
		mMaterial.mSpecular = XMFLOAT3(inSpecular.r, inSpecular.g, inSpecular.b);
		mMaterial.mShininess = inShininess;
	}

private:
	void BuildShaders(ID3D11Device* d3dDevice);
	void LoadTexture(ID3D11Device* d3dDevice, const aiScene* scene);


	Material	mMaterial;
	XMFLOAT4	mRotation;
	XMFLOAT3	mPosition;
	float		mRotationAmount;
	float		mScale;
	int			numIndices;
	UINT		mVertexBufferStride;

	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11Buffer				*perObjectVSCB;
	ID3D11Buffer				*perObjectPSCB;
	ID3D11InputLayout			*inputLayout;
	ID3D11VertexShader			*vertexShader;
	ID3D11PixelShader			*pixelShader;
	ID3D11SamplerState			*textureSampler;
	ID3D11ShaderResourceView	*texture0View;

};
