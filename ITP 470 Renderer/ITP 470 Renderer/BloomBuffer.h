#pragma once
#include <d3d11.h>

class BloomBuffer
{
public:
	BloomBuffer(ID3D11DeviceContext* inDeviceContext, int inWidth, int inHeight);
	~BloomBuffer();

private:
	BloomBuffer(const BloomBuffer& inBloomBuffer);
	BloomBuffer& operator=(const BloomBuffer& inBloomBuffer);

	unsigned int mWidth;
	unsigned int mHeight;
};

