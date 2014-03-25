#pragma once
#include <d3d11.h>

class GBuffer
{
public:
	GBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight);
	~GBuffer();

	ID3D11ShaderResourceView** GetShaderResourceViews() { return mSRV; }

	void BindBuffers(ID3D11DeviceContext* inDeviceContext);

private:
	GBuffer(const GBuffer& inGBuffer);
	GBuffer& operator=(const GBuffer& inGBuffer);

	unsigned int mWidth;
	unsigned int mHeight;

	ID3D11RenderTargetView* mRTV[3];
	ID3D11ShaderResourceView* mSRV[4];
	ID3D11Texture2D* mTex[4];
	ID3D11DepthStencilView* mDSV;

};

