#pragma once
#include <d3d11.h>

class ShadowMap
{
public:
	ShadowMap(ID3D11Device *ind3dDevice, unsigned int inWidth, unsigned int inHeight);
	~ShadowMap();

	ID3D11ShaderResourceView* GetDepthMapResourceView() { return mDepthMapResourceView; }

	void BindDepthStencilViewAndSetNullRenderTarget(ID3D11DeviceContext* ind3dDeviceContext);

private:
	ShadowMap(const ShadowMap& rhs);
	ShadowMap& operator=(const ShadowMap& rhs);

	unsigned int mWidth;
	unsigned int mHeight;

	ID3D11ShaderResourceView* mDepthMapResourceView;
	ID3D11DepthStencilView* mDepthMapStencilView;

	D3D11_VIEWPORT mViewport;
};

