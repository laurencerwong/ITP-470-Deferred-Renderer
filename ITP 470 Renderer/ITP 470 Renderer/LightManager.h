#pragma once

#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "MeshData.h"

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
	float mInnerRadius, mOuterRadius, mVelocity, pad1, pad2;

} PointLight;


class LightManager
{

public:
	LightManager();
	~LightManager();
	void Initialize(ID3D11Device *inDevice);
	void SetShaderConstant(ID3D11DeviceContext *inDeviceContext, PointLight &inPointLight);
	void SetShaderConstant(ID3D11DeviceContext *inDeviceContext, DirectionalLight &inDirectionalLight);

	void CreateDirectionalLight	(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition);
	void UpdateDirectionalLight	(const XMVECTOR &inPosition);
	void CreatePointLight		(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius);

	const MeshData* GetLightVolumeMesh() { return &mLightVolumeMesh; }

	void Update					(float dt);

	std::vector<PointLight>			const& GetPointLights()			{ return mPointLights; }
	std::vector<DirectionalLight>	const& GetDirectionalLights()	{ return mDirectionalLights; }

private:
	LightManager(const LightManager &inLightManager);

	std::vector<PointLight>			mPointLights;
	std::vector<DirectionalLight>	mDirectionalLights;
	MeshData mLightVolumeMesh;

	struct PerPointLightCBStruct
	{
		PointLight mPointlight;
	};
	struct PerDirLightCBStruct
	{
		DirectionalLight mDirlight;
	};
	ID3D11Buffer* mPerPointLightCB;
	ID3D11Buffer* mPerDirLightCB;
};

