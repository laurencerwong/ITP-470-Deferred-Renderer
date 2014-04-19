#pragma once
#include <d3d11.h>
#include <vector>

class MeshData
{
public:
	MeshData();
	~MeshData();

	template<class T>
	void Initialize(ID3D11Device *ind3dDevice, std::vector<T> &inVertexList, std::vector<UINT> &inIndexList);

	void SetVertexAndIndexBuffers(ID3D11DeviceContext *ind3dDeviceContext) const;

	ID3D11Buffer* GetVertexBuffer() { return mVertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() { return mIndexBuffer; }

private:
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;

	UINT mStride;
};

template<class T>
void MeshData::Initialize(ID3D11Device *ind3dDevice, std::vector<T> &inVertexList, std::vector<UINT> &inIndexList)
{
	mStride = sizeof(T);

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.ByteWidth = sizeof(T)* inVertexList.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &inVertexList[0];

	ind3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &mVertexBuffer);

	D3D11_BUFFER_DESC indicesBufferDesc;
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.ByteWidth = sizeof(UINT)* inIndexList.size();
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.CPUAccessFlags = 0;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = &inIndexList[0];

	ind3dDevice->CreateBuffer(&indicesBufferDesc, &indexInitData, &mIndexBuffer);
}

