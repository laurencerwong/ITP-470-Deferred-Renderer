#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "assimp/include/mesh.h"
#include "assimp/include/scene.h"



using namespace DirectX;


struct perObjectCBStruct
{
	XMMATRIX mWorld;
};

class DrawableObject
{
public:
	DrawableObject();

	DrawableObject(ID3D11Buffer *inVertexBuffer, ID3D11Buffer *inIndexBuffer, int inNumIndices, ID3D11Buffer *inConstantBuffer,
		ID3D11InputLayout *inLayout, UINT inVertexBufferStride, ID3D11VertexShader *inVertexShader,
		ID3D11PixelShader *inPixelShader, ID3D11SamplerState *inSamplerState, ID3D11ShaderResourceView *inShaderResourceView) :
		vertexBuffer(inVertexBuffer), indexBuffer(inIndexBuffer), numIndices(inNumIndices), perObjectConstantBuffer(inConstantBuffer),
		inputLayout(inLayout), mVertexBufferStride(inVertexBufferStride), vertexShader(inVertexShader),
		pixelShader(inPixelShader), textureSampler(inSamplerState), texture0View(inShaderResourceView),
		mPosition(0.0f, 0.0f, 0.0f),
		mRotationAmount(0.0f){};

	~DrawableObject();
	void Draw(ID3D11DeviceContext* d3dDeviceContext);
	void Update(float deltaTime);
	void LoadFromString(std::string const& file, ID3D11Device* d3dDevice);

	void SetVertexBuffer(ID3D11Buffer *inBuffer)		{ vertexBuffer = inBuffer; }
	ID3D11Buffer* GetVertexBuffer()						{ return vertexBuffer; }

	void SetIndexBuffer(ID3D11Buffer *inBuffer)			{ indexBuffer = inBuffer; }
	ID3D11Buffer* GetIndexBuffer()						{ return indexBuffer; }

	void SetConstantBuffer(ID3D11Buffer *inBuffer)		{ perObjectConstantBuffer = inBuffer; }
	ID3D11Buffer* GetConstantBuffer()					{ return perObjectConstantBuffer; }

	void SetInputLayout(ID3D11InputLayout *inLayout)	{ inputLayout = inLayout; }
	ID3D11InputLayout* GetInputLayout()					{ return inputLayout; }

	void SetVertexBufferStride(UINT inStride)			{ mVertexBufferStride = inStride; }
	UINT GetVertexBufferStride()						{ return mVertexBufferStride; }

	void SetVertexShader(ID3D11VertexShader *inVertexShader)	{ vertexShader = inVertexShader; }
	ID3D11VertexShader *GetVertexShader()				{ return vertexShader; }

	void SetPixelShader(ID3D11PixelShader *inPixelShader)	{ pixelShader = inPixelShader; }
	ID3D11PixelShader *GetPixelShader()					{ return pixelShader; }


private:
	void BuildShaders(ID3D11Device* d3dDevice);
	void LoadTexture(ID3D11Device* d3dDevice, const aiScene* scene);

	XMFLOAT4	mRotation;
	XMFLOAT3	mPosition;
	float		mRotationAmount;
	float		mScale;
	int			numIndices;
	UINT		mVertexBufferStride;

	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11Buffer				*perObjectConstantBuffer;
	ID3D11InputLayout			*inputLayout;
	ID3D11VertexShader			*vertexShader;
	ID3D11PixelShader			*pixelShader;
	ID3D11SamplerState			*textureSampler;
	ID3D11ShaderResourceView	*texture0View;

};
