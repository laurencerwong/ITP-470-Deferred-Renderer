#include "Renderer.h"
#include "DrawableObject.h"

Renderer::Renderer(HINSTANCE hInstance)
: D3DApp(hInstance), mCurrentViewMode(VIEW_MODE_FULL)
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
	lightManager->Initialize(md3dDevice);

	shaderManager = new ShaderManager(md3dDevice, md3dImmediateContext);
	loader = new SceneLoader(md3dDevice, shaderManager);

	int sponza = loader->LoadFile("sponza.obj");
	loader->GetDrawableObject(sponza)->SetScale(1.0f);
	//loader->GetDrawableObject(sponza)->SetPosition(XMFLOAT3(0.0f, 0.0f, 30.0f));
	//mSkybox = loader->LoadFile("skysphere.obj");
	//loader->GetDrawableObject(mSkybox)->SetScale(2.0f);
	//loader->LoadFile("temp2.obj");
	DeclareShaderConstants(md3dDevice);

	//init default lights
	lightManager->CreateDirectionalLight(XMFLOAT4(1.0f, 0.78f, 0.5f, 1.0f), XMFLOAT3(5.0f, -5.0f, 0.0f));


	lightManager->CreatePointLight(XMFLOAT4(1.0f, 1.0f, 0.9f, 1.0f), XMFLOAT3(0.0f, 5.0f, -5.0f), 1.0f, 8.0f);
	lightManager->CreatePointLight(XMFLOAT4(0.8f, 0.0f, 0.8f, 1.0f), XMFLOAT3(15.0f, 15.0f, -5.0f), 1.0f, 8.0f);
	lightManager->CreatePointLight(XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-5.0f, 20.0f, 0.0f), 1.0f, 36.0f);
	
	lightManager->CreatePointLight(XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 5.0f, 0.0f), 1.0f, 36.0f);

	shadowMap = new ShadowMap(md3dDevice, 2048, 2048);
	gBuffer = new GBuffer(md3dDevice, mScreenViewport.Width, mScreenViewport.Height);
	laBuffer = new LightAccumulationBuffer(md3dDevice, mScreenViewport.Width, mScreenViewport.Height);
	texturedQuad = new TexturedQuad(shaderManager);
	texturedQuad->Initialize(md3dDevice);
	deferredRenderTarget = new TexturedQuad(shaderManager);
	deferredRenderTarget->Initialize(md3dDevice);

	InitializeMiscShaders();

	//set the depth biasing to prevent shadow acne
	D3D11_RASTERIZER_DESC newRasterizerDesc;
	newRasterizerDesc.DepthBias = 10;
	newRasterizerDesc.DepthBiasClamp = 0.0f;
	newRasterizerDesc.SlopeScaledDepthBias = 1.0f;
	newRasterizerDesc.CullMode = D3D11_CULL_BACK;
	newRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	newRasterizerDesc.FrontCounterClockwise = false;
	newRasterizerDesc.DepthClipEnable = false;
	newRasterizerDesc.ScissorEnable = false;
	newRasterizerDesc.MultisampleEnable = false;
	newRasterizerDesc.AntialiasedLineEnable = false;

	md3dDevice->CreateRasterizerState(&newRasterizerDesc, &mNoShadowAcneState);
	

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
	//loader->GetDrawableObject(mSkybox)->SetPosition(camera->GetPosition());
	if (mUpdateLights)
	{
		lightManager->Update(dt);
	}
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

void Renderer::FillGBuffer()
{
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		shaderManager->SetVertexShader(object->GetVertexShader());
		object->UpdatePSConstantBuffer(md3dImmediateContext);
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->UpdateSamplerState(md3dImmediateContext);
		object->Draw(md3dImmediateContext);
	}
}

void Renderer::DrawDeferred()
{
	//Lighting to light accumulation buffer
	laBuffer->BindBuffers(md3dImmediateContext, mDepthStencilView);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFramePSDeferredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameDeferredPSStruct *cbDeferred = (perFrameDeferredPSStruct*)mappedResource.pData;
	cbDeferred->gAmbientColor = XMLoadFloat4(&XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	cbDeferred->gCamPos = XMLoadFloat3(&camera->GetPosition());
	md3dImmediateContext->Unmap(perFramePSDeferredBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSDeferredBuffer);

	shaderManager->SetPixelShader("point_lightingpassPS.cso");
	gBuffer->SetShaderResources(md3dImmediateContext);
	float blendFactor[4] = { 0.5, 0.5, 0.5, 0.5 };
	md3dImmediateContext->OMSetBlendState(*laBuffer->GetBlendState(), blendFactor, 0xffffffff);
	deferredRenderTarget->GetDraw()->UpdateSamplerState(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->GetMeshData()->SetVertexAndIndexBuffers(md3dImmediateContext);
	shaderManager->SetVertexShader("quadVS.cso");
	for (PointLight p : lightManager->GetPointLights())
	{
		lightManager->SetShaderConstant(md3dImmediateContext, p);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	md3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);

	//combine buffers
	SetBackBufferRenderTarget();
	md3dImmediateContext->Map(perFramePSCombinationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCombinationPSStruct *cbCombination = (perFrameCombinationPSStruct*)mappedResource.pData;
	cbCombination->gAmbientColor = XMLoadFloat4(&XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	md3dImmediateContext->Unmap(perFramePSCombinationBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSCombinationBuffer);

	shaderManager->SetPixelShader("lightBlendPS.cso");
	md3dImmediateContext->PSSetShaderResources(0, 1, &gBuffer->GetShaderResourceViews()[0]);
	md3dImmediateContext->PSSetShaderResources(1, 1, laBuffer->GetShaderResourceViews());
	deferredRenderTarget->GetDraw()->UpdateSamplerState(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->GetMeshData()->SetVertexAndIndexBuffers(md3dImmediateContext);
	shaderManager->SetVertexShader("quadVS.cso");
	md3dImmediateContext->DrawIndexed(6, 0, 0);
}

void Renderer::DrawPhong()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->RSSetState(mNoShadowAcneState);
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
		ID3D11ShaderResourceView* psShaderResourceViews = { shadowMap->GetDepthMapResourceView() };
		md3dImmediateContext->PSSetShaderResources(2, 1, &psShaderResourceViews);
		object->SetShadowTransform(XMLoadFloat4x4(&mShadowTransform));
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
	constantVSMatrix->mView = XMLoadFloat4x4(&mView);
	md3dImmediateContext->Unmap(perFrameVSConstantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &perFrameVSConstantBuffer);
}

void Renderer::UpdatePerFrameVSCBShadowMap()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFrameVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBStruct *constantVSMatrix = (perFrameCBStruct*)mappedResource.pData;
	constantVSMatrix->mProj = XMLoadFloat4x4(&mLightProj);
	constantVSMatrix->mView = XMLoadFloat4x4(&mLightView);
	md3dImmediateContext->Unmap(perFrameVSConstantBuffer, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &perFrameVSConstantBuffer);
}

void Renderer::DrawSceneToShadowMap(ShadowMap *inShadowMap)
{
	inShadowMap->BindDepthStencilViewAndSetNullRenderTarget(md3dImmediateContext);
	BuildShadowTransform();
	UpdatePerFrameVSCBShadowMap();
	DrawDepth();

	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	XMStoreFloat4x4(&mView, camera->GetViewMatrix());
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.1f, 1000.0f));
}


void Renderer::BuildShadowTransform()
{
	BoundingSphere sceneBoundingSphere = loader->GetBoundingSphere();

	XMVECTOR lightDir = XMLoadFloat3(&lightManager->GetDirectionalLights()[0].mDirection);
	XMVECTOR lightPos = -2.0f * sceneBoundingSphere.mRadius * lightDir;
	XMVECTOR targetPos = XMLoadFloat3(&sceneBoundingSphere.mCenter);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, up);

	XMStoreFloat4x4(&mLightView, lightView);

	XMFLOAT3 sphereCenterLS;
	XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, XMLoadFloat4x4(&mLightView)));

	float l = sphereCenterLS.x - sceneBoundingSphere.mRadius;
	float b = sphereCenterLS.y - sceneBoundingSphere.mRadius;
	float n = sphereCenterLS.z - sceneBoundingSphere.mRadius;
	float r = sphereCenterLS.x + sceneBoundingSphere.mRadius;
	float t = sphereCenterLS.y + sceneBoundingSphere.mRadius;
	float f = sphereCenterLS.z + sceneBoundingSphere.mRadius;

	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

	XMStoreFloat4x4(&mLightProj, lightProj);

	XMMATRIX transNDCtoTex(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMStoreFloat4x4(&mShadowTransform, lightView * lightProj * transNDCtoTex);
	
}

void Renderer::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//DrawSceneToShadowMap(shadowMap);
	gBuffer->BindBuffers(md3dImmediateContext);
	shaderManager->SetPixelShader("gbufferfill.cso");
	shaderManager->SetVertexShader("phongVS.cso");

	//HHHHHAAAACKTACULAR
	XMStoreFloat4x4(&mView, camera->GetViewMatrix());
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.1f, 1000.0f));
	UpdatePerFrameVSCB();
	FillGBuffer();

	switch (mCurrentViewMode)
	{
	case VIEW_MODE_FULL:
		DrawDeferred();
		//texturedQuad->GetDraw()->SetTexture(0, deferredRenderTarget->GetShaderResourceView(), nullptr);
		//texturedQuad->GetDraw()->SetPixelShader("quadPS.cso");
		//DrawPhong();
		break;
	case VIEW_MODE_SHADOW_DEPTH:
		//texturedQuad->GetDraw()->SetPixelShader("quadDepthPS.cso");
		texturedQuad->GetDraw()->SetTexture(0, shadowMap->GetDepthMapResourceView(), nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	case VIEW_MODE_DIFFUSE:
		texturedQuad->GetDraw()->SetTexture(0, gBuffer->GetShaderResourceViews()[0], nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	case VIEW_MODE_NORMAL:
		texturedQuad->GetDraw()->SetTexture(0, gBuffer->GetShaderResourceViews()[1], nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	case VIEW_MODE_SPECULAR:
		texturedQuad->GetDraw()->SetTexture(0, gBuffer->GetShaderResourceViews()[2], nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	case VIEW_MODE_DEPTH:
		texturedQuad->GetDraw()->SetTexture(0, gBuffer->GetShaderResourceViews()[3], nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	case VIEW_MODE_LIGHT_ACCUM:
		texturedQuad->GetDraw()->SetTexture(0, *laBuffer->GetShaderResourceViews(), nullptr);
		SetBackBufferRenderTarget();

		shaderManager->SetPixelShader(texturedQuad->GetDraw()->GetPixelShader());
		shaderManager->SetVertexShader(texturedQuad->GetDraw()->GetVertexShader());
		texturedQuad->GetDraw()->UpdateSamplerState(md3dImmediateContext);

		texturedQuad->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
		texturedQuad->GetDraw()->Draw(md3dImmediateContext);
		break;
	}

	ID3D11ShaderResourceView* unBindAllResources[16] = { 0 };
	md3dImmediateContext->PSSetShaderResources(0, 16, unBindAllResources);
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

	perFrameConstantBufferDesc.ByteWidth = sizeof(perFrameDeferredPSStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFramePSDeferredBuffer);

	perFrameConstantBufferDesc.ByteWidth = sizeof(perFrameCombinationPSStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFramePSCombinationBuffer);
}

void Renderer::SetBackBufferRenderTarget()
{
	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	return;
}

void Renderer::InitializeMiscShaders()
{
	shaderManager->AddPixelShader("depthPS.cso");

	D3D11_INPUT_ELEMENT_DESC depthInputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	shaderManager->AddVertexShader("depthVS.cso", depthInputLayout, 1);

	shaderManager->AddPixelShader("gbufferfill.cso");

	shaderManager->AddPixelShader("point_lightingpassPS.cso");
	shaderManager->AddPixelShader("lightBlendPS.cso");
}

void Renderer::OnKeyUp(WPARAM inKeyCode)
{
	switch (inKeyCode)
	{
	case L'u':
	case L'U':
		mUpdateObjects = !mUpdateObjects;
		break;
	case L'v':
	case L'V':
		mCurrentViewMode = static_cast<ViewMode>((mCurrentViewMode + 1) % 7);
		break;
	case L'l':
	case L'L':
		lightManager->UpdateDirectionalLight(XMLoadFloat3(&camera->GetTarget()) - XMLoadFloat3(&camera->GetPosition()));
		break;
	}
}

void Renderer::OnKeyDown(WPARAM inKeyCode)
{
	
}
