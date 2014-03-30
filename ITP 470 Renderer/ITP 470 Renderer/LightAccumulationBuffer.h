#pragma once
#include <d3d11.h>
//Write lighting values here before combining them with the ambient diffuse color
//XYZ = diffuse light
//W = specular power
class LightAccumulationBuffer
{
public:
	LightAccumulationBuffer(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight);
	~LightAccumulationBuffer();
	ID3D11ShaderResourceView** GetShaderResourceViews() { return &mSRV; }

	void BindBuffers(ID3D11DeviceContext* inDeviceContext, ID3D11DepthStencilView *inDSV);
	void SetShaderResources(ID3D11DeviceContext* inDeviceContext);
private:
	LightAccumulationBuffer(const LightAccumulationBuffer& inLightAccumulationBuffer);
	LightAccumulationBuffer& operator=(const LightAccumulationBuffer& inLightAccumulationBuffer);

	unsigned int mWidth;
	unsigned int mHeight;

	ID3D11RenderTargetView* mRTV;
	ID3D11ShaderResourceView* mSRV;
	ID3D11Texture2D* mTex;
};

