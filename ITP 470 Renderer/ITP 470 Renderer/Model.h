#pragma once

#include <d3d11.h>
#include "FrankLunaCode\d3dx11effect.h"
#include <DirectXMath.h>

using namespace DirectX;

struct Vertex1
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

class Model
{
public:
	Model();
	~Model();
	const ID3D11Buffer* GetVertexBuffer() const { return vertexBuffer; }
	const ID3D11Buffer* GetIndexBuffer() { return indexBuffer; }
	void SetVertexBuffer(ID3D11Buffer* newBuffer) { vertexBuffer = newBuffer; }
	void Init(ID3D11Device* d3dDevice);
	void Draw(ID3D11DeviceContext* d3dDeviceContext, ID3DX11EffectTechnique* technique);
private:

	void BuildVertexBuffer(ID3D11Device* d3dDevice);	

	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
};
