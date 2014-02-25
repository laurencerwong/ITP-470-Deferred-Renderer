#include "MeshData.h"
#include <vector>

MeshData::MeshData()
{
}


MeshData::~MeshData()
{
}

void MeshData::SetVertexAndIndexBuffers(ID3D11DeviceContext *ind3dDeviceContext)
{
	UINT offset = 0;

	ind3dDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &mStride, &offset);
	ind3dDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
}

