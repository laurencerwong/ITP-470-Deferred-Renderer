#include "Model.h"
#include "FrankLunaCode\d3dUtil.h"
#include <d3dcompiler.h>

Model::Model()
{

}

Model::~Model()
{

}

void Model::Draw(ID3D11DeviceContext* d3dDeviceContext, ID3DX11EffectTechnique* technique)
{
	UINT stride = sizeof(Vertex1);
	UINT offset = 0;
	d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	d3dDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	D3DX11_TECHNIQUE_DESC techDesc;
	technique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		technique->GetPassByIndex(p)->Apply(0, d3dDeviceContext);

		// 36 indices for the box.
		d3dDeviceContext->DrawIndexed(36, 0, 0);
	}
}

void Model::Init(ID3D11Device* d3dDevice)
{
	BuildVertexBuffer(d3dDevice);
}

void Model::BuildVertexBuffer(ID3D11Device* d3dDevice)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.ByteWidth = sizeof(Vertex1)* 8; //only need 8 since we are making a cube
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	Vertex1 vertices[] = {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::White) },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Black) },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Red) },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4((const float*)&Colors::Green) },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Blue) },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Yellow) },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Cyan) },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4((const float*)&Colors::Magenta) }
	};

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = vertices;

	d3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &vertexBuffer);

	UINT indices[] = {
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC indicesBufferDesc;
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.ByteWidth = sizeof(UINT)* 36;
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.CPUAccessFlags = 0;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = indices;

	d3dDevice->CreateBuffer(&indicesBufferDesc, &indexInitData, &indexBuffer);
}


