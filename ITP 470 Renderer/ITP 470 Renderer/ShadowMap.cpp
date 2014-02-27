#include "ShadowMap.h"
#include <assert.h>

ShadowMap::ShadowMap(ID3D11Device* ind3dDevice, unsigned int inWidth, unsigned int inHeight)
: mWidth(inWidth), mHeight(inHeight), mDepthMapResourceView(0), mDepthMapStencilView(0)
{
	mViewport.TopLeftX = 0.0f;
	mViewport.TopLeftY = 0.0f;
	mViewport.Height = static_cast<float>(mHeight);
	mViewport.Width = static_cast<float>(mWidth);
	mViewport.MinDepth = 0.0f;
	mViewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL |
		D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ID3D11Texture2D *depthMap = 0;
	HRESULT hr = ind3dDevice->CreateTexture2D(&texDesc, 0, &depthMap);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = ind3dDevice->CreateDepthStencilView(depthMap, &depthStencilViewDesc, &mDepthMapStencilView);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = texDesc.MipLevels;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	hr = ind3dDevice->CreateShaderResourceView(depthMap, &shaderResourceViewDesc, &mDepthMapResourceView);
	if (FAILED(hr))
	{
		assert(false);
	}

	depthMap->Release();
}


ShadowMap::~ShadowMap()
{
}

void ShadowMap::BindDepthStencilViewAndSetNullRenderTarget(ID3D11DeviceContext *ind3dDeviceContext)
{
	ind3dDeviceContext->RSSetViewports(1, &mViewport);

	ID3D11RenderTargetView* renderTargets[1] = { 0 };
	ind3dDeviceContext->OMSetRenderTargets(1, renderTargets, mDepthMapStencilView);
	
	ind3dDeviceContext->PSSetShader(nullptr, 0, 0);

	ind3dDeviceContext->ClearDepthStencilView(mDepthMapStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
