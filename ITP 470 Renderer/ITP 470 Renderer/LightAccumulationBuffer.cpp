#include "LightAccumulationBuffer.h"
#include <cassert>


LightAccumulationBuffer::LightAccumulationBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight)
{
	//Create Diffuse Resources
	D3D11_TEXTURE2D_DESC laTexDesc;
	laTexDesc.Width = inWidth;
	laTexDesc.Height = inHeight;
	laTexDesc.MipLevels = 1;
	laTexDesc.ArraySize = 1;
	laTexDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	laTexDesc.SampleDesc.Count = 1;
	laTexDesc.SampleDesc.Quality = 0;
	laTexDesc.Usage = D3D11_USAGE_DEFAULT;
	laTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	laTexDesc.CPUAccessFlags = 0;
	laTexDesc.MiscFlags = 0;

	HRESULT hr = inDevice->CreateTexture2D(&laTexDesc, 0, &mTex);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC laSRVDesc;
	laSRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	laSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	laSRVDesc.Texture2D.MostDetailedMip = 0;
	laSRVDesc.Texture2D.MipLevels = 1;
	hr = inDevice->CreateShaderResourceView(mTex, &laSRVDesc, &mSRV);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_RENDER_TARGET_VIEW_DESC laRTDesc;
	laRTDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	laRTDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	laRTDesc.Texture2D.MipSlice = 0;

	hr = inDevice->CreateRenderTargetView(mTex, &laRTDesc, &mRTV);
	if (FAILED(hr))
	{
		assert(false);
	}

	D3D11_BLEND_DESC laBlendDesc;
	laBlendDesc.AlphaToCoverageEnable = false;
	laBlendDesc.IndependentBlendEnable = false;
	laBlendDesc.RenderTarget[0].BlendEnable = true;
	laBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	laBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	laBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	laBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	laBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	laBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	laBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;// &~D3D11_COLOR_WRITE_ENABLE_ALPHA;
	inDevice->CreateBlendState(&laBlendDesc, &mBS);
}

LightAccumulationBuffer::~LightAccumulationBuffer()
{
}

void LightAccumulationBuffer::BindAndClearBuffers(ID3D11DeviceContext *inDeviceContext, ID3D11DepthStencilView *inDSV)
{
	float clearTargetColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	inDeviceContext->OMSetRenderTargets(1, &mRTV, inDSV);
	inDeviceContext->ClearDepthStencilView(inDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	inDeviceContext->ClearRenderTargetView(mRTV, clearTargetColor);
}

void LightAccumulationBuffer::BindBuffers(ID3D11DeviceContext *inDeviceContext, ID3D11DepthStencilView *inDSV)
{
	inDeviceContext->OMSetRenderTargets(1, &mRTV, inDSV);
	inDeviceContext->ClearDepthStencilView(inDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void LightAccumulationBuffer::SetShaderResources(ID3D11DeviceContext* inDeviceContext)
{
	inDeviceContext->PSSetShaderResources(0, 1, &mSRV);
}