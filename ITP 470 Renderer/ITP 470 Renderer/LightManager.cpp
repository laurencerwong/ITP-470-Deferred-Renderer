#include "LightManager.h"

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
	
	DirectionalLight newDirectionalLight;
	newDirectionalLight.mColor = inColor;
	XMStoreFloat3(&newDirectionalLight.mDirection, XMVector3Normalize(XMLoadFloat3(&inPosition)));
	XMStoreFloat4(&newDirectionalLight.mSpecularColor,  XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	mDirectionalLights.push_back(newDirectionalLight);
	
}

void LightManager::UpdateDirectionalLight(const XMVECTOR &inPosition)
{
	XMStoreFloat3(&mDirectionalLights[0].mDirection, XMVector3Normalize(inPosition));
}

void LightManager::CreatePointLight(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius)
{
	PointLight newPointLight;
	newPointLight.mDiffuseColor = inColor;
	newPointLight.mPosition = inPosition;
	newPointLight.mInnerRadius = inInnerRadius;
	newPointLight.mOuterRadius = inOuterRadius;
	newPointLight.mVelocity = 10.0f;
	XMStoreFloat4(&newPointLight.mSpecularColor, XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	mPointLights.push_back(newPointLight);

}

void LightManager::Update(float dt)
{
	for (auto &it : mPointLights)
	{
		if (it.mPosition.x < -12)
		{
			it.mPosition.x = -12;
			it.mVelocity *= -1.0;
		}
		if (it.mPosition.x > 12)
		{
			it.mPosition.x = 12;
			it.mVelocity *= -1.0;
		}
		it.mPosition.x += it.mVelocity * dt;
	}
}
