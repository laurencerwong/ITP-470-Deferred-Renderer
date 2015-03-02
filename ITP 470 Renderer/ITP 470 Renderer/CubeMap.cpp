#include "CubeMap.h"
#include <cassert>

CubeMap::CubeMap(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight)
{
	mHeight = inHeight;
	mWidth = inWidth;
	//Create Textures 
	mVP.TopLeftX = 0.0f;
	mVP.TopLeftY = 0.0f;
	mVP.Height = static_cast<float>(mHeight);
	mVP.Width = static_cast<float>(mWidth);
	mVP.MinDepth = 0.0f;
	mVP.MaxDepth = 1.0f;

	//TODO change this to use D3D11_TEXCUBE_SRV and update arraysize of texture2d_desc to cubemap

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = //D3D11_BIND_DEPTH_STENCIL |
		D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	HRESULT hr = inDevice->CreateTexture2D(&texDesc, 0, &mTex);
	if (FAILED(hr))
	{
		assert(false);
	}

	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texDesc.ArraySize = 1;
	texDesc.MiscFlags = 0;

	hr = inDevice->CreateTexture2D(&texDesc, 0, &mDepthTex);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	shaderResourceViewDesc.Texture2D.MipLevels = texDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.TextureCube.MipLevels = 1;
	shaderResourceViewDesc.TextureCube.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2DArray.ArraySize = 6;
	shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
	shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
	shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
	hr = inDevice->CreateShaderResourceView(mTex, &shaderResourceViewDesc, &mCubeSRV);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	hr = inDevice->CreateSamplerState(&samplerDesc, &mSS);
	if (FAILED(hr))
	{
		assert(false);
	}


	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	depthStencilViewDesc.Texture2DArray.ArraySize = 1;
	depthStencilViewDesc.Texture2DArray.FirstArraySlice = 0;
	depthStencilViewDesc.Texture2DArray.MipSlice = 0;
	hr = inDevice->CreateDepthStencilView(mDepthTex, &depthStencilViewDesc, &mDSV[0]);
	if (FAILED(hr))
	{
		assert(false);
	}
	for (int i = 0; i < 6; ++i)
	{


		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;//DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		shaderResourceViewDesc.Texture2D.MipLevels = texDesc.MipLevels;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2DArray.ArraySize = 1;
		shaderResourceViewDesc.Texture2DArray.FirstArraySlice = i;
		shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
		shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
		hr = inDevice->CreateShaderResourceView(mTex, &shaderResourceViewDesc, &mSRV[i]);
		if (FAILED(hr))
		{
			assert(false);
		}

		
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		renderTargetViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		renderTargetViewDesc.Texture2DArray.ArraySize = 1;
		renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
		renderTargetViewDesc.Texture2DArray.MipSlice = 0;
		hr = inDevice->CreateRenderTargetView(mTex, &renderTargetViewDesc, &mRTV[i]);
		if (FAILED(hr))
		{
				
			assert(false);
		}
		
	}

	D3D11_SAMPLER_DESC cubeSamplerDesc;
	ZeroMemory(&cubeSamplerDesc, sizeof(cubeSamplerDesc));
	cubeSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	cubeSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	cubeSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	cubeSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	cubeSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	cubeSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = inDevice->CreateSamplerState(&cubeSamplerDesc, &mSS);
	if (FAILED(hr))
	{

		assert(false);
	}
}


CubeMap::~CubeMap()
{
	for (int i = 0; i < 6; ++i)
	{
		mDSV[i]->Release();
		mSRV[i]->Release();
	}
	mTex->Release();
}

void CubeMap::BindBuffersAndSetNullRenderTarget(ID3D11DeviceContext* inDeviceContext, int inIndex)
{
	inDeviceContext->RSSetViewports(1, &mVP);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	inDeviceContext->OMSetRenderTargets(1, renderTargets, mDSV[inIndex]);
	inDeviceContext->PSSetShader(nullptr, 0, 0);
	inDeviceContext->ClearDepthStencilView(mDSV[inIndex], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void CubeMap::ClearBuffers(ID3D11DeviceContext* inDeviceContext)
{ 
	for (int i = 0; i < 6; ++i)
	{
		FLOAT clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		inDeviceContext->ClearRenderTargetView(mRTV[i], clearColor);
	}
}
void CubeMap::BindBuffersAndSetDepthView(ID3D11DeviceContext* inDeviceContext, ID3D11DepthStencilView* inDSV, int inIndex)
{
	inDeviceContext->RSSetViewports(1, &mVP);
	inDeviceContext->OMSetRenderTargets(1, &mRTV[inIndex], mDSV[0]);
	inDeviceContext->ClearDepthStencilView(mDSV[0], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//inDeviceContext->ClearRenderTargetView(mRTV[inIndex], clearColor);
}
