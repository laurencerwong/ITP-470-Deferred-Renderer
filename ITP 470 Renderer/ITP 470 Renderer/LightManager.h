#pragma once

#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

typedef struct DirectionalLight
{
	XMFLOAT4 mColor;
	XMFLOAT4 mSpecularColor;
	XMFLOAT3 mDirection;
	float mPad;

} DirectionalLight;

typedef struct PointLight
{
	XMFLOAT4 mDiffuseColor, mSpecularColor;
	XMFLOAT3 mPosition;
	float mInnerRadius, mOuterRadius, pad0, pad1, pad2;


} PointLight;


class LightManager
{

public:
	LightManager();
	LightManager(const LightManager &inLightManager);
	~LightManager();

	void CreateDirectionalLight	(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition);
	void UpdateDirectionalLight	(const XMVECTOR &inPosition);
	void CreatePointLight		(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius);

	void Update					(float dt);

	std::vector<PointLight>			const& GetPointLights()			{ return mPointLights; }
	std::vector<DirectionalLight>	const& GetDirectionalLights()	{ return mDirectionalLights; }

private:
	std::vector<PointLight>			mPointLights;
	std::vector<DirectionalLight>	mDirectionalLights;

};

