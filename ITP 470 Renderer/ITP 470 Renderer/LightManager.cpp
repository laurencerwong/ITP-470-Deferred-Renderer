#include "LightManager.h"
#include "SceneLoader.h"
#include <random>

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
	mPointLights.clear();
	mDirectionalLights.clear();
}

void LightManager::Initialize(ID3D11Device *inDevice)
{
	D3D11_BUFFER_DESC perLightConstantBufferDesc;
	perLightConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perLightConstantBufferDesc.ByteWidth = sizeof(PerPointLightCBStruct);
	perLightConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perLightConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perLightConstantBufferDesc.MiscFlags = 0;
	perLightConstantBufferDesc.StructureByteStride = 0;

	inDevice->CreateBuffer(&perLightConstantBufferDesc, NULL, &mPerPointLightCB);

	perLightConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perLightConstantBufferDesc.ByteWidth = sizeof(PerDirLightCBStruct);
	perLightConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perLightConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perLightConstantBufferDesc.MiscFlags = 0;
	perLightConstantBufferDesc.StructureByteStride = 0;

	inDevice->CreateBuffer(&perLightConstantBufferDesc, NULL, &mPerDirLightCB);

}

void LightManager::LoadLightVolumeMesh()
{

}

void LightManager::SetShaderConstant(ID3D11DeviceContext* inDeviceContext, PointLight &inPointLight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	inDeviceContext->Map(mPerPointLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PerPointLightCBStruct *perPointLightCB = (PerPointLightCBStruct*)mappedResource.pData;
	perPointLightCB->mPointlight = inPointLight;
	inDeviceContext->Unmap(mPerPointLightCB, 0);
	inDeviceContext->PSSetConstantBuffers(1, 1, &mPerPointLightCB);
}
void LightManager::SetShaderConstant(ID3D11DeviceContext* inDeviceContext, DirectionalLight &inDirLight)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	inDeviceContext->Map(mPerDirLightCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PerDirLightCBStruct *perDirLightCB = (PerDirLightCBStruct*)mappedResource.pData;
	perDirLightCB->mDirlight = inDirLight;
	inDeviceContext->Unmap(mPerDirLightCB, 0);
	inDeviceContext->PSSetConstantBuffers(1, 1, &mPerDirLightCB);
}

void LightManager::CreateDirectionalLight(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition)
{
	DirectionalLightContainer newDirectionalLightContainer;
	XMStoreFloat3(&newDirectionalLightContainer.mDirLightDesiredDir, XMVector3Normalize(XMLoadFloat3(&inPosition)));
	XMStoreFloat3(&newDirectionalLightContainer.mDirLightPreviousDir, XMVector3Normalize(XMLoadFloat3(&inPosition)));
	newDirectionalLightContainer.mDirLightDesiredDirLerp = 0.0f;

	DirectionalLight newDirectionalLight;
	XMStoreFloat3(&newDirectionalLight.mDirection, XMVector3Normalize(XMLoadFloat3(&inPosition)));
	XMStoreFloat4(&newDirectionalLight.mSpecularColor,  XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	newDirectionalLight.mColor = inColor;
	newDirectionalLightContainer.mDirectionalLight = newDirectionalLight;

	mDirectionalLights.push_back(newDirectionalLightContainer);
}

void LightManager::UpdateDirectionalLight(const XMVECTOR &inPosition)
{
	mDirectionalLights[0].mDirLightPreviousDir = mDirectionalLights[0].mDirectionalLight.mDirection;
	XMStoreFloat3(&mDirectionalLights[0].mDirLightDesiredDir, XMVector3Normalize(inPosition));
	mDirectionalLights[0].mDirLightDesiredDirLerp = 0.0f;
}

void LightManager::CreatePointLight(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius, bool inShadowEnabled)
{
	PointLightContainer newPointLightContainer;
	PointLight newPointLight;
	newPointLight.mDiffuseColor = inColor;
	newPointLight.mPosition = inPosition;
	newPointLight.mInnerRadius = inInnerRadius;
	newPointLight.mOuterRadius = inOuterRadius;
	newPointLight.mVelocity = 10.0f;
	XMStoreFloat4(&newPointLight.mSpecularColor, XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	newPointLightContainer.mPointLight = newPointLight;
	newPointLightContainer.mLightVolume = &mPointLightVolumeMesh;
	newPointLightContainer.mShadowEnabled = inShadowEnabled;
	mPointLights.push_back(newPointLightContainer);

}

void LightManager::CreateRandomPointLight(const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius, bool inShadowEnabled)
{
	float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	float g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	float b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
	XMFLOAT4 randomColor = { r, g, b, 1.0f };
	XMStoreFloat4(&randomColor, XMVectorSaturate(XMLoadFloat4(&randomColor)));
	CreatePointLight(randomColor, inPosition, inInnerRadius, inOuterRadius, inShadowEnabled);
}

void LightManager::Update(float dt)
{
	for (auto &it : mPointLights)
	{
		if (it.mPointLight.mPosition.x < -12)
		{
			it.mPointLight.mPosition.x = -12;
			it.mPointLight.mVelocity *= -1.0;
		}
		if (it.mPointLight.mPosition.x > 12)
		{
			it.mPointLight.mPosition.x = 12;
			it.mPointLight.mVelocity *= -1.0;
		}
		it.mPointLight.mPosition.x += it.mPointLight.mVelocity * dt;
	}
	for (auto &it : mDirectionalLights)
	{
		it.mDirLightDesiredDirLerp += dt;
		XMVECTOR prevDirLightDir = XMLoadFloat3(&it.mDirLightPreviousDir);
		XMVECTOR desDirLightDir = XMLoadFloat3(&it.mDirLightDesiredDir);
		XMStoreFloat3(&it.mDirectionalLight.mDirection, XMVectorLerp(prevDirLightDir, desDirLightDir, it.mDirLightDesiredDirLerp / 4.0f));
		if (it.mDirLightDesiredDirLerp > 4.0f)
		{
			it.mDirLightDesiredDir.x *= -1;
			UpdateDirectionalLight(XMLoadFloat3(&it.mDirLightDesiredDir));
		}
	}
}
