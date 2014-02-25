#pragma once

#include "DrawableObject.h"
#include "ShaderManager.h"

class TexturedQuad
{
public:
	TexturedQuad(ShaderManager* inShaderManager);
	~TexturedQuad();

	void Initialize(ID3D11Device *ind3dDevice);
	void Resize(ID3D11Device *ind3dDevice, int inWidth, int inHeight);
	void SetAsRenderTarget(ID3D11DeviceContext* ind3dDeviceContext, ID3D11DepthStencilView* inDepthStencilView);
	void SetTexture(ID3D11ShaderResourceView* inTexture) { mShaderResourceView = inTexture; }

	ID3D11ShaderResourceView* GetShaderResourceView() { return mShaderResourceView; }

	DrawableObject* GetDraw() { return mDraw; }

private:
	ShaderManager *mShaderManager;
	DrawableObject *mDraw;

	ID3D11Texture2D *mRenderTargetTexture;
	ID3D11RenderTargetView *mRenderTargetView;
	ID3D11ShaderResourceView *mShaderResourceView;
	ID3D11SamplerState *textureSampler;
};

