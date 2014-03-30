#pragma once
#include <d3d11.h>

//GBuffer Class for deferred rendering
//Contains 5 Resources:
//0 - Diffuse
//1 - Normal
//2 - Specular
//3 - Position 
//4 - Depth
class GBuffer
{
public:
	GBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight);
	~GBuffer();

	ID3D11ShaderResourceView** GetShaderResourceViews() { return mSRV; }

	void BindBuffers(ID3D11DeviceContext* inDeviceContext);
	void SetShaderResources(ID3D11DeviceContext* inDeviceContext);

private:
	GBuffer(const GBuffer& inGBuffer);
	GBuffer& operator=(const GBuffer& inGBuffer);

	unsigned int mWidth;
	unsigned int mHeight;

	ID3D11RenderTargetView* mRTV[4];
	ID3D11ShaderResourceView* mSRV[5];
	ID3D11Texture2D* mTex[5];
	ID3D11DepthStencilView* mDSV;

};

