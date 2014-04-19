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

typedef struct DirectionalLightContainer
{
	DirectionalLight mDirectionalLight;

	XMFLOAT3 mDirLightDesiredDir;
	XMFLOAT3 mDirLightPreviousDir;
	float mDirLightDesiredDirLerp;
} DirectionalLightContainer;

typedef struct PointLight
{
	XMFLOAT4 mDiffuseColor, mSpecularColor;
	XMFLOAT3 mPosition;
	float mInnerRadius, mOuterRadius, mVelocity, pad1, pad2;

} PointLight;

typedef struct PointLightContainer
{
	PointLight mPointLight;
	MeshData* mLightVolume;
	bool mShadowEnabled;

} PointLightContainer;


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
	void CreatePointLight		(const XMFLOAT4 &inColor, const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius, bool inShadowEnabled);
	void CreateRandomPointLight (const XMFLOAT3 &inPosition, float inInnerRadius, float inOuterRadius, bool inShadowEnabled);

	void SetLightVolumeMesh(const MeshData& inMesh, int inNumIndices) { mPointLightVolumeMesh = inMesh; mPointLightVolumeMeshIndexCount = inNumIndices; }
	const MeshData* GetLightVolumeMesh() { return &mPointLightVolumeMesh; }
	int GetLightVolumeIndexCount() { return mPointLightVolumeMeshIndexCount; }

	void Update					(float dt);

	std::vector<PointLightContainer>		const& GetPointLights()			{ return mPointLights; }
	std::vector<DirectionalLightContainer>	const& GetDirectionalLights()	{ return mDirectionalLights; }

private:
	LightManager(const LightManager &inLightManager);
	void LoadLightVolumeMesh();

	std::vector<PointLightContainer>		mPointLights;
	std::vector<DirectionalLightContainer>	mDirectionalLights;
	MeshData mPointLightVolumeMesh;
	int mPointLightVolumeMeshIndexCount;


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

