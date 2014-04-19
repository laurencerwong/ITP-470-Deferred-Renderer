#include "GBuffer.h"
#include <cassert>

GBuffer::GBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight)
: mWidth(inWidth)
, mHeight(inHeight)
{
	//0 == Diffuse
	//1 == Normal
	//2 == Specular
	//3 == Depth

	//Create Diffuse Resources
	D3D11_TEXTURE2D_DESC diffuseTexDesc;
	diffuseTexDesc.Width = inWidth;
	diffuseTexDesc.Height = inHeight;
	diffuseTexDesc.MipLevels = 1;
	diffuseTexDesc.ArraySize = 1;
	diffuseTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	diffuseTexDesc.SampleDesc.Count = 1;
	diffuseTexDesc.SampleDesc.Quality = 0;
	diffuseTexDesc.Usage = D3D11_USAGE_DEFAULT;
	diffuseTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	diffuseTexDesc.CPUAccessFlags = 0;
	diffuseTexDesc.MiscFlags = 0;

	HRESULT hr = inDevice->CreateTexture2D(&diffuseTexDesc, 0, &mTex[0]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC diffuseSRVDesc;
	diffuseSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	diffuseSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	diffuseSRVDesc.Texture2D.MostDetailedMip = 0;
	diffuseSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex[0], &diffuseSRVDesc, &mSRV[0]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC diffuseRTDesc;
	diffuseRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	diffuseRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	diffuseRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex[0], &diffuseRTDesc, &mRTV[0]);
	if (FAILED(hr))
	{
		assert(false);
	}

	//Create Normal Resources
	D3D11_TEXTURE2D_DESC normalTexDesc;
	normalTexDesc.Width = inWidth;
	normalTexDesc.Height = inHeight;
	normalTexDesc.MipLevels = 1;
	normalTexDesc.ArraySize = 1;
	normalTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalTexDesc.SampleDesc.Count = 1;
	normalTexDesc.SampleDesc.Quality = 0;
	normalTexDesc.Usage = D3D11_USAGE_DEFAULT;
	normalTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	normalTexDesc.CPUAccessFlags = 0;
	normalTexDesc.MiscFlags = 0;

	hr = inDevice->CreateTexture2D(&normalTexDesc, 0, &mTex[1]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC normalSRVDesc;
	normalSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	normalSRVDesc.Texture2D.MostDetailedMip = 0;
	normalSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex[1], &normalSRVDesc, &mSRV[1]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC normalRTDesc;
	normalRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	normalRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	normalRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex[1], &normalRTDesc, &mRTV[1]);
	if (FAILED(hr))
	{
		assert(false);
	}

	//Create Specular Resources
	D3D11_TEXTURE2D_DESC specularTexDesc;
	specularTexDesc.Width = inWidth;
	specularTexDesc.Height = inHeight;
	specularTexDesc.MipLevels = 1;
	specularTexDesc.ArraySize = 1;
	specularTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	specularTexDesc.SampleDesc.Count = 1;
	specularTexDesc.SampleDesc.Quality = 0;
	specularTexDesc.Usage = D3D11_USAGE_DEFAULT;
	specularTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	specularTexDesc.CPUAccessFlags = 0;
	specularTexDesc.MiscFlags = 0;

	hr = inDevice->CreateTexture2D(&specularTexDesc, 0, &mTex[2]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC specularSRVDesc;
	specularSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;;
	specularSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	specularSRVDesc.Texture2D.MostDetailedMip = 0;
	specularSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex[2], &specularSRVDesc, &mSRV[2]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC specularRTDesc;
	specularRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	specularRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	specularRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex[2], &specularRTDesc, &mRTV[2]);
	if (FAILED(hr))
	{
		assert(false);
	}


	//Create Position Resources
	D3D11_TEXTURE2D_DESC positionTexDesc;
	positionTexDesc.Width = inWidth;
	positionTexDesc.Height = inHeight;
	positionTexDesc.MipLevels = 1;
	positionTexDesc.ArraySize = 1;
	positionTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionTexDesc.SampleDesc.Count = 1;
	positionTexDesc.SampleDesc.Quality = 0;
	positionTexDesc.Usage = D3D11_USAGE_DEFAULT;
	positionTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	positionTexDesc.CPUAccessFlags = 0;
	positionTexDesc.MiscFlags = 0;

	hr = inDevice->CreateTexture2D(&positionTexDesc, 0, &mTex[3]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC positionSRVDesc;
	positionSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	positionSRVDesc.Texture2D.MostDetailedMip = 0;
	positionSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex[3], &positionSRVDesc, &mSRV[3]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC positionRTDesc;
	positionRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	positionRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	positionRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex[3], &positionRTDesc, &mRTV[3]);
	if (FAILED(hr))
	{
		assert(false);
	}

	//Create Depth Resources
	D3D11_TEXTURE2D_DESC depthTexDesc;
	depthTexDesc.Width = inWidth;
	depthTexDesc.Height = inHeight;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	hr = inDevice->CreateTexture2D(&depthTexDesc, 0, &mTex[4]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex[4], &depthSRVDesc, &mSRV[4]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC depthRTDesc;
	depthRTDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	depthRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex[4], &depthRTDesc, &mRTV[4]);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SAMPLER_DESC ssDesc;
	ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ssDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	ssDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	ssDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ssDesc.MipLODBias = 0;
	ssDesc.MaxAnisotropy = 16;

	hr = inDevice->CreateSamplerState(&ssDesc, &mSS);
	if (FAILED(hr))
	{
		assert(false);
	}
}


GBuffer::~GBuffer()
{
}

void GBuffer::BindBuffers(ID3D11DeviceContext* inDeviceContext, ID3D11DepthStencilView* inDepthStencilView)
{
	float clearTargetColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	inDeviceContext->OMSetRenderTargets(5, mRTV, inDepthStencilView);
	inDeviceContext->ClearDepthStencilView(inDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	inDeviceContext->ClearRenderTargetView(mRTV[0], clearTargetColor);
	inDeviceContext->ClearRenderTargetView(mRTV[1], clearTargetColor);
	inDeviceContext->ClearRenderTargetView(mRTV[2], clearTargetColor);
	inDeviceContext->ClearRenderTargetView(mRTV[3], clearTargetColor);
	inDeviceContext->ClearRenderTargetView(mRTV[4], clearTargetColor);
}

void GBuffer::SetShaderResources(ID3D11DeviceContext* inDeviceContext)
{
	inDeviceContext->PSSetShaderResources(0, 5, mSRV);
}
