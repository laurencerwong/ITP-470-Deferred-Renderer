#pragma once
#include <d3d11.h>

/*
GBuffer Class for deferred rendering
Contains 5 Resources:
0 - Diffuse
1 - Normal
2 - Specular
3 - Position - Unneeded
4 - Depth

Depth buffer technique
http://mynameismjp.wordpress.com/2009/03/10/reconstructing-position-from-depth/
*/
class GBuffer
{
public:
	GBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight);
	~GBuffer();

	ID3D11ShaderResourceView** GetShaderResourceViews() { return mSRV; }
	ID3D11SamplerState** GetSamplerState() { return &mSS; }

	void BindBuffers(ID3D11DeviceContext* inDeviceContext, ID3D11DepthStencilView* inDepthStencilView);
	void SetShaderResources(ID3D11DeviceContext* inDeviceContext);

private:
	GBuffer(const GBuffer& inGBuffer);
	GBuffer& operator=(const GBuffer& inGBuffer);

	unsigned int mWidth;
	unsigned int mHeight;

	ID3D11SamplerState* mSS;
	ID3D11RenderTargetView* mRTV[5];
	ID3D11ShaderResourceView* mSRV[5];
	ID3D11Texture2D* mTex[5];
};

