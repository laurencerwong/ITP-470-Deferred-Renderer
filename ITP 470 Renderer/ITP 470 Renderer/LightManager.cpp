#include "LightManager.h"

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
	mPointLights.clear();
	mDirectionalLights.clear();
}

void LightManager::CreateDirectionalLight(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition)
{
	
	DirectionalLight newDirectionalLight;
	newDirectionalLight.mColor = inColor;
	newDirectionalLight.mDirection = inPosition;
	XMStoreFloat4(&newDirectionalLight.mSpecularColor,  XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	mDirectionalLights.push_back(newDirectionalLight);
	
}

void LightManager::CreatePointLight(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius)
{
	PointLight newPointLight;
	newPointLight.mDiffuseColor = inColor;
	newPointLight.mPosition = inPosition;
	newPointLight.mInnerRadius = inInnerRadius;
	newPointLight.mOuterRadius = inOuterRadius;
	XMStoreFloat4(&newPointLight.mSpecularColor, XMLoadFloat4(&inColor) + XMLoadFloat4(&XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f)));
	mPointLights.push_back(newPointLight);

}

void LightManager::Update(float dt)
{
	for (auto it : mPointLights)
	{
	//	it.Update(dt);
	}
}
