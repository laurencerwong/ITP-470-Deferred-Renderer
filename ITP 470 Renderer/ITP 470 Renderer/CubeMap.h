#pragma once
#include <d3d11.h>

/*
6 Textures that represent a cube of textures
-------------------------
|	0	|	1	|	2	|
-------------------------
|	3	|	4	|	5	|
-------------------------
*/
class CubeMap
{
public:
	CubeMap(ID3D11Device *inDevice, unsigned int inWidth, unsigned int inHeight);
	~CubeMap();

	ID3D11ShaderResourceView** GetShaderResourceViews() { return mSRV; }
	ID3D11ShaderResourceView** GetCubeShaderResourceView() { return &mCubeSRV; }
	ID3D11SamplerState** GetComparisonSamplerState() { return &mSS;	}

	void BindBuffersAndSetNullRenderTarget(ID3D11DeviceContext* inDeviceContext, int inIndex);
	void ClearBuffers(ID3D11DeviceContext* inDeviceContext);
	void BindBuffersAndSetDepthView(ID3D11DeviceContext* inDeviceContet, ID3D11DepthStencilView* inDSV, int inIndex);
	void SetShaderResources(ID3D11DeviceContext* inDeviceContext);
private:
	CubeMap(const CubeMap& inRHS);
	CubeMap& operator=(const CubeMap& inRHS);

	unsigned int mWidth;
	unsigned int mHeight;

	
	ID3D11DepthStencilView* mDSV[6];
	ID3D11ShaderResourceView* mSRV[6];
	ID3D11RenderTargetView* mRTV[6];
	ID3D11ShaderResourceView* mCubeSRV;
	ID3D11SamplerState *mSS;
	ID3D11Texture2D *mTex;
	//cube map needs own depth texture since the dimensions of the cube faces does not equal the renderer's 
	ID3D11Texture2D *mDepthTex;
	D3D11_VIEWPORT mVP;
};

