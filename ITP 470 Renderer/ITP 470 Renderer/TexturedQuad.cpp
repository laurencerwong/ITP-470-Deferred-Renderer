#include "TexturedQuad.h"
#include <vector>
#include "Vertex.h"

TexturedQuad::TexturedQuad(ShaderManager* inShaderManager) : 
mShaderManager(inShaderManager)
{
}


TexturedQuad::~TexturedQuad()
{
}

void InitializeVerticesAndIndices(std::vector<VertexPosTex> &inVertices, std::vector<UINT> &inIndices)
{
	inVertices.resize(4);
	inVertices[0].Pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	inVertices[0].Tex0 = XMFLOAT2(0.0f, 1.0f);

	inVertices[1].Pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	inVertices[1].Tex0 = XMFLOAT2(0.0f, 0.0f);

	inVertices[2].Pos = XMFLOAT3(1.0f, 1.0f, 0.0f);
	inVertices[2].Tex0 = XMFLOAT2(1.0f, 0.0f);

	inVertices[3].Pos = XMFLOAT3(1.0f, -1.0f, 0.0f);
	inVertices[3].Tex0 = XMFLOAT2(1.0f, 1.0f);

	inIndices.resize(6);
	inIndices[0] = 0;
	inIndices[1] = 1;
	inIndices[2] = 2;
	inIndices[3] = 0;
	inIndices[4] = 2;
	inIndices[5] = 3;
}

void TexturedQuad::Initialize(ID3D11Device *ind3dDevice)
{
	std::vector<VertexPosTex> vertices;
	std::vector<UINT> indices;

	InitializeVerticesAndIndices(vertices, indices);

	mDraw = new DrawableObject();
	mDraw->GetMeshData()->Initialize(ind3dDevice, vertices, indices);

	D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	mDraw->SetVertexShader(mShaderManager->AddVertexShader("quadVS.cso", inputLayout, 2));
	mShaderManager->AddPixelShader("quadDepthPS.cso");
	mDraw->SetPixelShader(mShaderManager->AddPixelShader("quadPS.cso"));

	D3D11_SAMPLER_DESC textureSamplerDesc;
	ZeroMemory(&textureSamplerDesc, sizeof(textureSamplerDesc));
	textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	textureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	textureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ID3D11SamplerState *newSamplerState;
	ind3dDevice->CreateSamplerState(&textureSamplerDesc, &newSamplerState);
	mDraw->SetSamplerState(newSamplerState);

	D3D11_TEXTURE2D_DESC textureDesc;

	textureDesc.Width = 800;
	textureDesc.Height = 600;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Quality = 0;

	ind3dDevice->CreateTexture2D(&textureDesc, NULL, &mRenderTargetTexture);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
	renderTargetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	ind3dDevice->CreateRenderTargetView(mRenderTargetTexture, &renderTargetDesc, &mRenderTargetView);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderViewDesc;
	shaderViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderViewDesc.Texture2D.MostDetailedMip = 0;
	shaderViewDesc.Texture2D.MipLevels = 1;

	ind3dDevice->CreateShaderResourceView(mRenderTargetTexture, &shaderViewDesc, &mShaderResourceView);

	ID3D11Buffer *perObjectConstantBuffer;
	D3D11_BUFFER_DESC perObjectConstantBufferDesc;
	perObjectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectConstantBufferDesc.ByteWidth = sizeof(perObjectCBVSStruct);
	perObjectConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectConstantBufferDesc.MiscFlags = 0;
	perObjectConstantBufferDesc.StructureByteStride = 0;
	ind3dDevice->CreateBuffer(&perObjectConstantBufferDesc, NULL, &perObjectConstantBuffer);
	mDraw->SetVSConstantBuffer(perObjectConstantBuffer);

	mDraw->AddTexture(GetShaderResourceView(), nullptr);
	mDraw->AddPart(0, 0, 6, 0);
}

void TexturedQuad::SetAsRenderTarget(ID3D11DeviceContext* ind3dDeviceContext, ID3D11DepthStencilView* inDepthStencilView)
{
	mDraw->SetTexture(0, GetShaderResourceView(), nullptr);
	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	ind3dDeviceContext->PSSetShaderResources(0, 1, nullSRVs); // unbinds the texture so that you don't read and write at the same time
	ind3dDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, inDepthStencilView);

	return;
}

void TexturedQuad::Resize(ID3D11Device *ind3dDevice, int inWidth, int inHeight)
{

}
