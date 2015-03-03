struct PointLight
{
	float4 mColor;
	float4 mSpecularColor;
	float3 mPosition;
	float mInnerRadius, mOuterRadius;
};
cbuffer perLightCB : register(b0)
{
	PointLight pointLight;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 posWorld : TEXCOORD0;
};

float VecToDepth(float3 inVec)
{
	float3 vabs = abs(inVec);
	float highComp = max(vabs.x, max(vabs.y, vabs.z));

	float NormZComp = (pointLight.mOuterRadius+0.01f) / (pointLight.mOuterRadius-0.01f) - (2*pointLight.mOuterRadius*0.01f)/(pointLight.mOuterRadius-0.01f)/highComp;
	return (NormZComp + 1.0f) * 0.5f;
}

float4 main(PS_IN input) : SV_TARGET
{
	float3 vToLight = pointLight.mPosition - input.posWorld.xyz;
	float depth = length(vToLight) / pointLight.mOuterRadius;
	return float4(depth, depth, depth, 1.0f);// *gColor;
}