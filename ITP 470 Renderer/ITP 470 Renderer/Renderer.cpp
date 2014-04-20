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

	MeshData lightVolume;
	int numIndices = loader->LoadModel("lightvolume.obj", lightVolume);
	lightManager->SetLightVolumeMesh(lightVolume, numIndices);
	//init default lights
	lightManager->CreateDirectionalLight(XMFLOAT4(1.0f, 0.78f, 0.5f, 1.0f), XMFLOAT3(5.0f, -5.0f, 0.0f));

	lightManager->CreatePointLight(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(-6.0f, 5.0f, 0.0f), 0.01f, 18.0f, true);
	//lightManager->CreateRandomPointLight(XMFLOAT3(-6.0f, 2.0f, 6.0f), 1.0f, 12.0f, true);
	/*
	lightManager->CreateRandomPointLight(XMFLOAT3(0.0f, 2.0f, 6.0f), 1.0f, 6.0f, true);
	lightManager->CreateRandomPointLight(XMFLOAT3(6.0f, 2.0f, 6.0f), 1.0f, 6.0f, true);
	lightManager->CreateRandomPointLight(XMFLOAT3(12.0f, 2.0f, 6.0f), 1.0f, 6.0f, true);
	lightManager->CreateRandomPointLight(XMFLOAT3(-12.0f, 2.0f, -6.0f), 1.0f, 6.0f, true);
	*/
	//lightManager->CreateRandomPointLight(XMFLOAT3(-6.0f, 2.0f, -6.0f), 1.0f, 12.0f, true);
	/*
	lightManager->CreateRandomPointLight(XMFLOAT3(0.0f, 2.0f, -6.0f), 1.0f, 6.0f, true);
	lightManager->CreateRandomPointLight(XMFLOAT3(6.0f, 2.0f, -6.0f), 1.0f, 6.0f, true);
	lightManager->CreateRandomPointLight(XMFLOAT3(12.0f, 2.0f, -6.0f), 1.0f, 6.0f, true);
	*/
	
	

	shadowMap = new ShadowMap(md3dDevice, 2048, 2048);
	gBuffer = new GBuffer(md3dDevice, static_cast<unsigned int>(mScreenViewport.Width), static_cast<unsigned int>(mScreenViewport.Height));
	cubeShadowMap = new CubeMap(md3dDevice, static_cast<unsigned int>(512), static_cast<unsigned int>(512));
	laBuffer = new LightAccumulationBuffer(md3dDevice, static_cast<unsigned int>(mScreenViewport.Width), static_cast<unsigned int>(mScreenViewport.Height));
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
	// Setup a raster description which turns off back face culling.
	newRasterizerDesc.AntialiasedLineEnable = false;
	newRasterizerDesc.CullMode = D3D11_CULL_NONE;
	newRasterizerDesc.DepthBias = 0;
	newRasterizerDesc.DepthBiasClamp = 0.0f;
	newRasterizerDesc.DepthClipEnable = true;
	newRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	newRasterizerDesc.FrontCounterClockwise = false;
	newRasterizerDesc.MultisampleEnable = false;
	newRasterizerDesc.ScissorEnable = false;
	newRasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the no culling rasterizer state.
	md3dDevice->CreateRasterizerState(&newRasterizerDesc, &mNoCulling);

	D3D11_DEPTH_STENCIL_DESC newDepthStencilDesc;
	newDepthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_LESS;
	newDepthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	newDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	newDepthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	newDepthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	newDepthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	newDepthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	newDepthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	newDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	newDepthStencilDesc.StencilWriteMask = 0xFF;
	newDepthStencilDesc.StencilReadMask = 0xFF;
	newDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	newDepthStencilDesc.StencilEnable = true;
	newDepthStencilDesc.DepthEnable = true;
	HRESULT result = md3dDevice->CreateDepthStencilState(&newDepthStencilDesc, &mAlwaysLessDepthStencilState);
	if (FAILED(result))
	{
		assert(false);
	}

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
}

void Renderer::OnMouseMoveRaw(WPARAM btnState, RAWMOUSE &mouse)
{
	camera->SetMouseCoords(static_cast<float>(mouse.lLastX), static_cast<float>(mouse.lLastY));
	if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
	{
		camera->UpdateMouseWheel(mouse.usButtonData);
	}
}

void Renderer::DrawOmniDepth(int inIndex, PointLight& inPointLight)
{
	shaderManager->SetVertexShader("omniDirShadowVS.cso");
	shaderManager->SetPixelShader("omniDirShadowPS.cso");

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFramePSPointBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFramePointLightShadowPSStruct*cbDeferred = (perFramePointLightShadowPSStruct*)mappedResource.pData;
	cbDeferred->gPointLight = inPointLight;
	md3dImmediateContext->Unmap(perFramePSPointBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSPointBuffer);
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->DrawWithoutTextures(md3dImmediateContext);
	}
}

void Renderer::DrawDepth()
{
	shaderManager->SetVertexShader("depthVS.cso");
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->DrawWithoutTextures(md3dImmediateContext);
	}
}

void Renderer::FillGBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFramePSDeferredFillBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameDeferredFillStruct*cbDeferred = (perFrameDeferredFillStruct*)mappedResource.pData;
	cbDeferred->gFarPlane = XMLoadFloat4(&XMFLOAT4(1000.0f, 1.0f, 1.0f, 1.0f));
	md3dImmediateContext->Unmap(perFramePSDeferredFillBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSDeferredFillBuffer);
	ID3D11ShaderResourceView* psShaderResourceViews = { shadowMap->GetDepthMapResourceView() };
	md3dImmediateContext->PSSetShaderResources(2, 1, &psShaderResourceViews);
	for (DrawableObject* object : loader->GetDrawableObjects())
	{
		shaderManager->SetVertexShader(object->GetVertexShader());
		object->UpdatePSConstantBuffer(md3dImmediateContext);
		object->SetShadowTransform(XMLoadFloat4x4(&mShadowTransform));
		object->UpdateVSConstantBuffer(md3dImmediateContext);
		object->UpdateSamplerState(md3dImmediateContext);
		object->Draw(md3dImmediateContext);
	}
}

void Renderer::DrawDeferred()
{
	//Lighting to light accumulation buffer
	//Steps - 1.Stencil buffer 2. Lighting
	laBuffer->BindAndClearBuffers(md3dImmediateContext, mDepthStencilView);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFramePSDeferredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameDeferredPSStruct *cbDeferred = (perFrameDeferredPSStruct*)mappedResource.pData;
	cbDeferred->gShadowTransform = XMLoadFloat4x4(&mShadowTransform);
	cbDeferred->gAmbientColor = XMLoadFloat4(&XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
	cbDeferred->gCamPos = XMLoadFloat3(&camera->GetPosition());
	md3dImmediateContext->Unmap(perFramePSDeferredBuffer, 0);
	md3dImmediateContext->PSSetConstantBuffers(0, 1, &perFramePSDeferredBuffer);
	gBuffer->SetShaderResources(md3dImmediateContext);
	float blendFactor[4] = { 0.5, 0.5, 0.5, 0.5 };
	md3dImmediateContext->OMSetBlendState(*laBuffer->GetBlendState(), blendFactor, 0xffffffff);
	deferredRenderTarget->GetDraw()->UpdateSamplerState(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->UpdateVSConstantBuffer(md3dImmediateContext);
	deferredRenderTarget->GetDraw()->GetMeshData()->SetVertexAndIndexBuffers(md3dImmediateContext);
	shaderManager->SetVertexShader("quadVS.cso");
	//shaderManager->SetVertexShader("point_lightingpassVS.cso");
	shaderManager->SetPixelShader("point_lightingpassPS.cso");
	md3dImmediateContext->RSSetState(mNoShadowAcneState);
	md3dImmediateContext->PSSetSamplers(0, 1, gBuffer->GetSamplerState());
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	shaderManager->SetPixelShader("directional_lightingpassPS.cso");
	for (DirectionalLightContainer d : lightManager->GetDirectionalLights())
	{
		lightManager->SetShaderConstant(md3dImmediateContext, d.mDirectionalLight);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	for (PointLightContainer pc : lightManager->GetPointLights())
	{
/*		PointLight pl = pc.mPointLight;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		md3dImmediateContext->Map(perFramePSDeferredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		perFrameDeferredPSStruct *cbDeferred = (perFrameDeferredPSStruct*)mappedResource.pData;
		cbDeferred->gShadowTransform = XMMatrixScaling(pl.mOuterRadius, pl.mOuterRadius, pl.mOuterRadius) * XMMatrixIdentity() * XMMatrixTranslationFromVector(XMLoadFloat3(&pl.mPosition));
		md3dImmediateContext->Unmap(perFramePSDeferredBuffer, 0);
		md3dImmediateContext->VSSetConstantBuffers(1, 1, &perFramePSDeferredBuffer);
		
		lightManager->GetLightVolumeMesh()->SetVertexAndIndexBuffers(md3dImmediateContext);
		*/
		if (pc.mShadowEnabled)
		{
			md3dImmediateContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
			DrawDepthForPoint(pc.mPointLight);
			md3dImmediateContext->OMSetBlendState(*laBuffer->GetBlendState(), blendFactor, 0xffffffff);
			shaderManager->SetVertexShader("quadVS.cso");
			shaderManager->SetPixelShader("point_lightingpassShadowPS.cso");
			laBuffer->BindBuffers(md3dImmediateContext, mDepthStencilView);
			deferredRenderTarget->GetDraw()->GetMeshData()->SetVertexAndIndexBuffers(md3dImmediateContext);
			md3dImmediateContext->PSSetShaderResources(5, 1, cubeShadowMap->GetCubeShaderResourceView());
			md3dImmediateContext->PSSetSamplers(1, 1, cubeShadowMap->GetComparisonSamplerState());
		}
		else
		{
			shaderManager->SetPixelShader("point_lightingpassPS.cso");
		}
		lightManager->SetShaderConstant(md3dImmediateContext, pc.mPointLight);
		md3dImmediateContext->DrawIndexed(6, 0, 0);
		//md3dImmediateContext->DrawIndexed(lightManager->GetLightVolumeIndexCount(), 0, 0);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	md3dImmediateContext->RSSetState(0);
//	shaderManager->SetVertexShader("quadVS.cso");
	
	
	//md3dImmediateContext->OMSetDepthStencilState(mDefaultDepthStencilState, *mDefaultDepthStencilStateRef);
	//md3dImmediateContext->OMSetDepthStencilState(nullptr, 0);
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
	constantPSStruct->gDirLight = lightManager->GetDirectionalLights()[0].mDirectionalLight;
	constantPSStruct->gPointLight[0] = lightManager->GetPointLights()[0].mPointLight;
	constantPSStruct->gPointLight[1] = lightManager->GetPointLights()[1].mPointLight;
	constantPSStruct->gPointLight[2] = lightManager->GetPointLights()[2].mPointLight;
	constantPSStruct->gPointLight[3] = lightManager->GetPointLights()[3].mPointLight;
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
	XMVECTOR frustrumCenter = XMLoadFloat3(&camera->GetPosition()) + (camera->GetForward() * 1000.0f);
	float screenHeight = mScreenViewport.Height;
	float screenWidth = mScreenViewport.Width;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(perFrameVSConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perFrameCBStruct *constantVSMatrix = (perFrameCBStruct*)mappedResource.pData;
	constantVSMatrix->mProj = XMLoadFloat4x4(&mProj);
	constantVSMatrix->mView = XMLoadFloat4x4(&mView);
	constantVSMatrix->gFarFrustrumCorners[0] = frustrumCenter + XMVectorSet(-screenWidth / 2.0f, screenHeight / 2.0f, 0.0f, 0.0f);
	constantVSMatrix->gFarFrustrumCorners[1] = frustrumCenter + XMVectorSet(screenWidth / 2.0f, screenHeight / 2.0f, 0.0f, 0.0f);
	constantVSMatrix->gFarFrustrumCorners[2] = frustrumCenter + XMVectorSet(-screenWidth / 2.0f, -screenHeight / 2.0f, 0.0f, 0.0f);
	constantVSMatrix->gFarFrustrumCorners[3] = frustrumCenter + XMVectorSet(screenWidth / 2.0f, -screenHeight / 2.0f, 0.0f, 0.0f);
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
	/*
	cubeShadowMap->ClearBuffers(md3dImmediateContext);
	for (int i = 0; i < 6; ++i)
	{
		cubeShadowMap->BindBuffersAndSetDepthView(md3dImmediateContext, mDepthStencilView, i);
		//cubeShadowMap->BindBuffersAndSetNullRenderTarget(md3dImmediateContext, i);
		BuildShadowTransform(lightManager->GetPointLights()[0].mPointLight, i);
		UpdatePerFrameVSCBShadowMap();
		//DrawDepth();
		DrawOmniDepth(i);
	}
	*/
	
	inShadowMap->BindDepthStencilViewAndSetNullRenderTarget(md3dImmediateContext);
	BuildShadowTransform(lightManager->GetDirectionalLights()[0].mDirectionalLight);
	UpdatePerFrameVSCBShadowMap();
	DrawDepth();

	md3dImmediateContext->RSSetState(0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	XMStoreFloat4x4(&mView, camera->GetViewMatrix());
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.01f, 1000.0f));
}

void Renderer::DrawDepthForPoint(PointLight &inLight)
{
	//Use point light shadow shader to draw in the distance of the pixel from the point light
	cubeShadowMap->ClearBuffers(md3dImmediateContext);
	for (int i = 0; i < 6; ++i)
	{
		cubeShadowMap->BindBuffersAndSetDepthView(md3dImmediateContext, mDepthStencilView, i);
		//cubeShadowMap->BindBuffersAndSetNullRenderTarget(md3dImmediateContext, i);
		BuildShadowTransform(inLight, i);
		UpdatePerFrameVSCBShadowMap();
		//DrawDepth();
		DrawOmniDepth(i, inLight);
	}
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
	XMStoreFloat4x4(&mView, camera->GetViewMatrix());
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.01f, 1000.0f));
}

void Renderer::BuildShadowTransform(const PointLight &inPointLight, int inFaceNum)
{
	XMFLOAT3 LIGHT_DIRS[] = { XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 1.0f),
	XMFLOAT3(0.0f, 0.0f, -1.0f)};
	XMFLOAT3 LIGHT_UP[] = { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f),
		XMFLOAT3(0.0f, 1.0f, 0.0f) };

	XMVECTOR lightDir = XMLoadFloat3(&LIGHT_DIRS[inFaceNum]);
	XMVECTOR lightPos = XMLoadFloat3(&inPointLight.mPosition);
	XMVECTOR targetPos = lightPos + lightDir;
	XMVECTOR up = XMLoadFloat3(&LIGHT_UP[inFaceNum]);

	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, targetPos, up);
	XMStoreFloat4x4(&mLightView, lightView);

	//Aspect ratio is 1.0f since the texture is always square
	XMMATRIX lightProj = XMMatrixPerspectiveFovLH(MathHelper::Pi/2.0f, 1.0f, 0.1f, inPointLight.mOuterRadius);
	XMStoreFloat4x4(&mLightProj, lightProj);


	/*
	XMMATRIX transNDCtoTex(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	XMStoreFloat4x4(&mShadowTransform, lightView * lightProj * transNDCtoTex);
	*/
}

void Renderer::BuildShadowTransform(const DirectionalLight &inDirectionalLight)
{
	BoundingSphere sceneBoundingSphere = loader->GetBoundingSphere();

	XMVECTOR lightDir = XMLoadFloat3(&inDirectionalLight.mDirection);
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

	DrawSceneToShadowMap(shadowMap);
	//DrawDepthForPoint(lightManager->GetPointLights()[0].mPointLight);
	gBuffer->BindBuffers(md3dImmediateContext, mDepthStencilView);
	shaderManager->SetPixelShader("gbufferfill.cso");
	shaderManager->SetVertexShader("phongVSShadowMap.cso");

	//HHHHHAAAACKTACULAR
	XMStoreFloat4x4(&mView, camera->GetViewMatrix());
	XMStoreFloat4x4(&mProj, XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 0.1f, 1000.0f));
	shaderManager->SetPixelShader("gbufferfill.cso");
	shaderManager->SetVertexShader("phongVSShadowMap.cso");
	UpdatePerFrameVSCB();
	FillGBuffer();

	switch (mCurrentViewMode)
	{
	case VIEW_MODE_FULL:
		DrawDeferred();
		break;
	case VIEW_MODE_SHADOW_DEPTH:
		texturedQuad->GetDraw()->SetTexture(0, cubeShadowMap->GetShaderResourceViews()[mCurrentCubeMap], nullptr);
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

	perFrameConstantBufferDesc.ByteWidth = sizeof(perFrameDeferredFillStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFramePSDeferredFillBuffer);

	perFrameConstantBufferDesc.ByteWidth = sizeof(perFramePointLightShadowPSStruct);
	perFrameConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perFrameConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perFrameConstantBufferDesc.MiscFlags = 0;
	perFrameConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perFrameConstantBufferDesc, NULL, &perFramePSPointBuffer);

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
	shaderManager->AddVertexShader("omniDirShadowVS.cso", depthInputLayout, 1);
	shaderManager->AddVertexShader("point_lightingpassVS.cso", depthInputLayout, 1);

	shaderManager->AddPixelShader("gbufferfill.cso");

	shaderManager->AddPixelShader("point_lightingpassPS.cso");
	shaderManager->AddPixelShader("point_lightingpassShadowPS.cso");
	shaderManager->AddPixelShader("directional_lightingpassPS.cso");
	shaderManager->AddPixelShader("lightBlendPS.cso");
	shaderManager->AddPixelShader("omniDirShadowPS.cso");
}

void Renderer::OnKeyUp(WPARAM inKeyCode)
{
	switch (inKeyCode)
	{
	case L's':
	case L'S':
		mCurrentCubeMap = (mCurrentCubeMap + 1) % 6;
		break;
	case L'1':
		mUpdateLights = !mUpdateLights;
		break;
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
