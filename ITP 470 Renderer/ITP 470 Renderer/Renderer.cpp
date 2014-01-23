#include "Renderer.h"
#include <d3dcompiler.h>

Renderer::Renderer(HINSTANCE hInstance)
: D3DApp(hInstance)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMVECTOR pos = XMVectorSet(0, 0, -5.0f, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	box = new Model();
}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	if (!D3DApp::Init())
		return false;

	box->Init(md3dDevice);
	BuildEffect(md3dDevice);
	BuildVertexDescription(md3dDevice);
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
}

void Renderer::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	// Set constants
	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX worldViewProj = world*view*proj;

	mWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	box->Draw(md3dImmediateContext, mTechnique);

	HR(mSwapChain->Present(0, 0));
}

void Renderer::BuildVertexDescription(ID3D11Device* d3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	d3dDevice->CreateInputLayout(vertex1Desc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
}

void Renderer::BuildEffect(ID3D11Device* d3dDevice)
{
	DWORD shaderFlags = 0;

#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob *compiledShader = 0;
	ID3D10Blob* compilationMsgs = 0;

	D3DCompileFromFile(L"Effects/color.fx", NULL, NULL, NULL, "fx_5_0", shaderFlags, 0, &compiledShader, &compilationMsgs);

	if (compilationMsgs != 0)
	{
		MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(compilationMsgs);
	}

	D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), 0, d3dDevice, &mEffect);

	ReleaseCOM(compiledShader);

	mTechnique = mEffect->GetTechniqueByName("ColorTech");
	mWorldViewProj = mEffect->GetVariableByName("gWorldViewProj")->AsMatrix();

}
