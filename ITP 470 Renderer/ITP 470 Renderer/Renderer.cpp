#include "Renderer.h"
#include "DrawableObject.h"

Renderer::Renderer(HINSTANCE hInstance)
: D3DApp(hInstance)
{
	lightManager = new LightManager();
	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mProj, I);
	camera = new Camera();

}

Renderer::~Renderer()
{
}

bool Renderer::Init()
{
	if (!D3DApp::Init())
		return false;
	loader = new SceneLoader(md3dDevice);

	loader->LoadFile("sponza.obj");
	//loader->LoadFile("temp2.obj");
	DeclareShaderConstants(md3dDevice);

	//init default lights
	lightManager->CreateDirectionalLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(20.0f, -5.0f, 0.0f));


	lightManager->CreatePointLight(XMFLOAT4(1.0f, 1.0f, 0.8f, 1.0f), XMFLOAT3(0.0f, 5.0f, 0.0f), 1.0f, 36.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 10.0f, 0.0f), 1.0f, 36.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-5.0f, 0.0f, 0.0f), 1.0f, 36.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), 1.0f, 36.0f);

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
	if (mUpdateObjects)
	{
		for (DrawableObject* object : loader->GetDrawableObjects())
		{
			object->Update(dt);
		}
	}
	lightManager->Update(dt);
	int size = sizeof(DirectionalLight);
	camera->Update(dt);
}

void Renderer::OnMouseMoveRaw(WPARAM btnState, RAWMOUSE &mouse)
{
	camera->SetMouseCoords(static_cast<float>(mouse.lLastX), static_cast<float>(mouse.lLastY));
	if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
	{
		camera->UpdateMouseWheel(mouse.usButtonData);
	}
}

void Renderer::DrawDepthStencil()
{
	assert(md3dImmediateContext);
	assert(mSwapChain); 

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	

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
	constantPSStruct->gDirLight = lightManager->GetDirectionalLights()[0];
	constantPSStruct->gPointLight[0] = lightManager->GetPointLights()[0];
	constantPSStruct->gPointLight[1] = lightManager->GetPointLights()[1];
	constantPSStruct->gPointLight[2] = lightManager->GetPointLights()[2];
	constantPSStruct->gPointLight[3] = lightManager->GetPointLights()[3];
	constantPSStruct->gAmbientColor = XMLoadFloat4(&XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	constantPSStruct->gCamPos = XMLoadFloat3(&camera->GetPosition());

	md3dImmediateContext->Unmap(perFramePSConstantBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSConstantBuffer);
	
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		object->Draw(md3dImmediateContext);
	}

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

void Renderer::CreateDepthStencilState(ID3D11Device* d3dDevice)
{
	D3D11_DEPTH_STENCIL_DESC noDoubleBlendDesc;
	noDoubleBlendDesc.DepthEnable		= true;
	noDoubleBlendDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ALL;
	noDoubleBlendDesc.DepthFunc			= D3D11_COMPARISON_LESS;
	noDoubleBlendDesc.StencilEnable		= true;
	noDoubleBlendDesc.StencilReadMask	= 0xff;
	noDoubleBlendDesc.StencilWriteMask	= 0xff;

	noDoubleBlendDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilDepthFailOp	= D3D11_STENCIL_OP_KEEP;
	noDoubleBlendDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_INCR;
	noDoubleBlendDesc.FrontFace.StencilFunc			= D3D11_COMPARISON_EQUAL;

	//don't need to worry about noDoubleBlendDesc.BackFace

	d3dDevice->CreateDepthStencilState(&noDoubleBlendDesc, &mNoDoubleBlendDSS);
}
