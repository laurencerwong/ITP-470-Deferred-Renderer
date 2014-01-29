#include "Renderer.h"
#include "FileReaderWriter.h"


struct colorVSStruct
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
};

Renderer::Renderer(HINSTANCE hInstance)
: D3DApp(hInstance)
{
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	XMVECTOR pos = XMVectorSet(0, 0, 0, 1.0f);
	XMVECTOR target = XMVectorSet(0.0f, 0.0f, 10.0f, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mWorld, XMMatrixTranslation(0.0f, 0.0f, 10.0f));

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);

	box = new DrawableObject();
}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	if (!D3DApp::Init())
		return false;

	box->Init(md3dDevice);
	BuildShaders(md3dDevice);
//	BuildVertexDescription(md3dDevice);
	return true;
}

void Renderer::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//XMMATRIX P = XMMatrixOrthographicLH(800, 600, 0.1, 1000.0);
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

	md3dImmediateContext->VSSetShader(colorVS, 0, 0);
	md3dImmediateContext->PSSetShader(colorPS, 0, 0);

	// Set constants


	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	colorVSStruct *constantMatrix = (colorVSStruct*)mappedResource.pData;
	constantMatrix->world = XMLoadFloat4x4(&mWorld);
	constantMatrix->view = XMLoadFloat4x4(&mView);
	constantMatrix->proj = XMLoadFloat4x4(&mProj);
	md3dImmediateContext->Unmap(constantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	box->Draw(md3dImmediateContext, mTechnique);

	HR(mSwapChain->Present(0, 0));
}

void Renderer::BuildVertexDescription(ID3D11Device* d3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC passDesc;
	mTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	d3dDevice->CreateInputLayout(vertex1Desc, 2, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout);
}

void Renderer::BuildShaders(ID3D11Device* d3dDevice)
{
	shaderData *vertexShader, *pixelShader;
	vertexShader = FileReaderWriter::ReadShader("colorVS.cso");
	pixelShader = FileReaderWriter::ReadShader("colorPS.cso");

	d3dDevice->CreateVertexShader(vertexShader->shaderByteData, vertexShader->size, nullptr, &colorVS);

	d3dDevice->CreatePixelShader(pixelShader->shaderByteData, pixelShader->size, nullptr, &colorPS);

	md3dImmediateContext->VSSetShader(colorVS, 0, 0);
	md3dImmediateContext->PSSetShader(colorPS, 0, 0);

	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	d3dDevice->CreateInputLayout(vertex1Desc, 2, vertexShader->shaderByteData, vertexShader->size, &mInputLayout);

	md3dImmediateContext->IASetInputLayout(mInputLayout);


	D3D11_BUFFER_DESC colorVSConstantBufferDesc;
	colorVSConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	colorVSConstantBufferDesc.ByteWidth = sizeof(colorVSStruct);
	colorVSConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	colorVSConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	colorVSConstantBufferDesc.MiscFlags = 0;
	colorVSConstantBufferDesc.StructureByteStride = 0;

	md3dDevice->CreateBuffer(&colorVSConstantBufferDesc, NULL, &constantBuffer);

	delete vertexShader;
	delete pixelShader;
}
