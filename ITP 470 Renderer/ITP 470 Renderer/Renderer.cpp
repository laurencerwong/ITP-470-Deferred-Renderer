#include "Renderer.h"



Renderer::Renderer(HINSTANCE hInstance)
: D3DApp(hInstance)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mProj, I);
	camera = new Camera();
	box = new DrawableObject();
}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	if (!D3DApp::Init())
		return false;

	box->LoadFromString("temp.obj", md3dDevice);
	DeclareShaderConstants(md3dDevice);
	return true;
}

void Renderer::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Renderer::UpdateScene(float dt)
{
	box->Update(dt);
	//camera->Update(dt);
}

void Renderer::OnMouseMoveRaw(WPARAM btnState, long x, long y)
{
	camera->SetMouseCoords(static_cast<float>(x), static_cast<float>(y));
}

void Renderer::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set constants
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFrameVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBStruct *constantVSMatrix = (perFrameCBStruct*)mappedResource.pData;
	constantVSMatrix->mProj = XMLoadFloat4x4(&mProj);
	constantVSMatrix->mView = camera->GetViewMatrix();
	md3dImmediateContext->Unmap(perFrameVSConstantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &perFrameVSConstantBuffer);

	md3dImmediateContext->Map(perFramePSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBPSStruct *constantPSStruct = (perFrameCBPSStruct*)mappedResource.pData;
	constantPSStruct->gAmbientColor = XMLoadFloat4(&XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	constantPSStruct->gLightColor = XMLoadFloat4(&XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
	constantPSStruct->gLightDir = XMVector3Normalize(XMLoadFloat3(&XMFLOAT3(1.0f, 0.0f, 0.0f)));
	md3dImmediateContext->Unmap(perFramePSConstantBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSConstantBuffer);

	box->Draw(md3dImmediateContext);

	HR(mSwapChain->Present(0, 0));
}

void Renderer::DeclareShaderConstants(ID3D11Device* d3dDevice)
{
	//declare constant buffer description
	D3D11_BUFFER_DESC perFrameConstantBufferDesc;
	perFrameConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perFrameConstantBufferDesc.ByteWidth = sizeof(perFrameCBStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFrameVSConstantBuffer);

	perFrameConstantBufferDesc.ByteWidth = sizeof(perFrameCBPSStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFramePSConstantBuffer);
}
