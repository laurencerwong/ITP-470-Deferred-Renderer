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
	shaderManager = new ShaderManager(md3dDevice, md3dImmediateContext);
	loader = new SceneLoader(md3dDevice, shaderManager);

	int sponza = loader->LoadFile("sponza.obj");
	loader->GetDrawableObject(sponza)->SetScale(1.0f);
	//loader->GetDrawableObject(sponza)->SetPosition(XMFLOAT3(0.0f, 0.0f, 30.0f));
	mSkybox = loader->LoadFile("skysphere.obj");
	loader->GetDrawableObject(mSkybox)->SetScale(2.0f);
	//loader->LoadFile("temp2.obj");
	DeclareShaderConstants(md3dDevice);

	//init default lights
	lightManager->CreateDirectionalLight(XMFLOAT4(1.0f, 0.78f, 0.5f, 1.0f), XMFLOAT3(5.0f, -5.0f, 0.0f));


	lightManager->CreatePointLight(XMFLOAT4(1.0f, 1.0f, 0.9f, 1.0f), XMFLOAT3(0.0f, 5.0f, -5.0f), 1.0f, 8.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.8f, 0.0f, 0.8f, 1.0f), XMFLOAT3(15.0f, 15.0f, -5.0f), 1.0f, 8.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-5.0f, -100.0f, 0.0f), 1.0f, 36.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, -100.0f, 0.0f), 1.0f, 36.0f);

	shadowMap = new ShadowMap(md3dDevice, 2048, 2048);
	texturedQuad = new TexturedQuad(shaderManager);
	texturedQuad->Initialize(md3dDevice);

	InitializeMiscShaders();

	return true;
}

void Renderer::OnResize()
{
	D3DApp::OnResize();
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.1f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Renderer::UpdateScene(float dt)
{
	//mUpdateObjects = true;
	if (mUpdateObjects)
	{
		for (DrawableObject* object : loader->GetDrawableObjects())
		{
			object->Update(dt);
		}
	}
	loader->GetDrawableObject(mSkybox)->SetPosition(camera->GetPosition());
	lightManager->Update(dt);
	camera->Update(dt);
	BuildShadowTransform();
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

void Renderer::DrawDepth()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	//shaderManager->SetPixelShader("depthPS.cso");
	shaderManager->SetVertexShader("depthVS.cso");
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->DrawWithoutTextures(md3dImmediateContext);
	}
}

void Renderer::DrawPhong()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

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
		shaderManager->SetPixelShader(object->GetPixelShader());
		shaderManager->SetVertexShader(object->GetVertexShader());
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->UpdatePSConstantBuffer(md3dImmediateContext);
		object->UpdateSamplerState(md3dImmediateContext);
		object->Draw(md3dImmediateContext);
	}
}

void Renderer::UpdatePerFrameVSCB()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFrameVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBStruct *constantVSMatrix = (perFrameCBStruct*)mappedResource.pData;
	constantVSMatrix->mProj = XMLoadFloat4x4(&mProj);
	//constantVSMatrix->mProj = XMMatrixOrthographicLH(64, 48, 0.1f, 500.0f);
	constantVSMatrix->mView = camera->GetViewMatrix();
	//constantVSMatrix->mView = XMLoadFloat4x4(&mView);
	md3dImmediateContext->Unmap(perFrameVSConstantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &perFrameVSConstantBuffer);
}

void Renderer::DrawSceneToShadowMap(ShadowMap *inShadowMap)
{
	inShadowMap->BindDepthStencilViewAndSetNullRenderTarget(md3dImmediateContext);

	DrawDepth();
}

void Renderer::BuildShadowTransform()
{
	BoundingSphere sceneBoundingSphere = loader->GetBoundingSphere();

	XMVECTOR lightDir = XMLoadFloat3(&lightManager->GetDirectionalLights()[0].mDirection);
	XMVECTOR lightPos = -2.0f * sceneBoundingSphere.mRadius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&sceneBoundingSphere.mCenter);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&mView, XMMatrixLookAtLH(lightPos, targetPos, up));
}

void Renderer::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UpdatePerFrameVSCB();

	DrawSceneToShadowMap(shadowMap);

	
	// Set constants
	/*texturedQuad->SetAsRenderTarget(md3dImmediateContext, mDepthStencilView);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFrameVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBStruct *constantVSMatrix = (perFrameCBStruct*)mappedResource.pData;
	constantVSMatrix->mProj = XMLoadFloat4x4(&mProj);
	//constantVSMatrix->mProj = XMMatrixOrthographicLH(64, 48, 0.1f, 500.0f);
	constantVSMatrix->mView = camera->GetViewMatrix();
	//constantVSMatrix->mView = XMLoadFloat4x4(&mView);
	md3dImmediateContext->Unmap(perFrameVSConstantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &perFrameVSConstantBuffer);

	DrawPhong();*/
	//DrawDepth();
	SetBackBufferRenderTarget();


	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	texturedQuad->GetDraw()->SetTexture(0, shadowMap->GetDepthMapResourceView(), nullptr);
	shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
	shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
	texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

	texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
	texturedQuad->GetDraw()->Draw(md3dImmediateContext);

	ID3D11ShaderResourceView* unBindAllResources[16] = { 0 };
	md3dImmediateContext->PSSetShaderResources(0, 1, unBindAllResources);
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

void Renderer::SetBackBufferRenderTarget()
{
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	return;
}

void Renderer::InitializeMiscShaders()
{
	shaderManager->AddPixelShader("depthPS.cso");

	D3D11_INPUT_ELEMENT_DESC depthInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	shaderManager->AddVertexShader("depthVS.cso", depthInputLayout, 1);
}

