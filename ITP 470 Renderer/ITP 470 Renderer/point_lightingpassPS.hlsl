Texture2D gDiffuseTex	: register(t0);
Texture2D gNormalTex	: register(t1);
Texture2D gSpecularTex	: register(t2);
Texture2D gPositionTex	: register(t3);
Texture2D gDepthTex		: register(t4);
TextureCube gShadowMap;

#define BETWEEN_0_1(num) (min(max((num), 0.0), 1.0))

SamplerState linearTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
	AddressW = Wrap;
};

SamplerComparisonState ShadowMapSampler
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
	AddressW = Clamp;
	ComparisonFunc = LESS_EQUAL;
};

struct PointLight
{
	float4 mColor;
	float4 mSpecularColor;
	float3 mPosition;
	float mInnerRadius, mOuterRadius;
};


cbuffer perFrameCB : register(b0)
{
	float4 gAmbientColor;
	float3 gCamPos;
};

cbuffer perLightCB : register(b1)
{
	PointLight pointLight;
};

struct PixelLightIn
{
	float4 pos : SV_POSITION;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 frustCorner : TEXCOORD0;
	float2 tex : TEXCOORD1;
};

float2 texFromPosition(float4 position)
{
	return (position / float4(800, 600, 1, 1)).xy;
}

float3 posFromDepth(float2 inTex, float3 inFrustrumVec)
{
	float depth = gDepthTex.Sample(linearTextureSampler, inTex).r;
	return depth * inFrustrumVec;

}

float VecToDepth(float3 inVec)
{
	float3 vabs = abs(inVec);
	float highComp = max(vabs.x, max(vabs.y, vabs.z));

	float NormZComp = (pointLight.mOuterRadius+0.01f) / (pointLight.mOuterRadius-0.01f) - (2*pointLight.mOuterRadius*0.01f)/(pointLight.mOuterRadius-0.01f)/highComp;
	return (NormZComp + 1.0f) * 0.5f;
}
float4 main(PixelIn input) : SV_TARGET0
{
	
	float4 diffuse = gDiffuseTex.Sample(linearTextureSampler, input.tex);
	float4 normal = normalize(gNormalTex.Sample(linearTextureSampler, input.tex));
	float4 specular = gSpecularTex.Sample(linearTextureSampler, input.tex);
	float4 position = gPositionTex.Sample(linearTextureSampler, input.tex);
	float4 depth = gDepthTex.Sample(linearTextureSampler, input.tex);

	float3 pixToCam = normalize(gCamPos - position.xyz);
	float3 pixToLight = pointLight.mPosition - position.xyz;
	float distToLight = length(pixToLight);
	float lerpAmount = 0.0f;
	float4 output = float4(0.0, 0.0, 0.0, 0.0);


		//Diffuse lighting
	lerpAmount = BETWEEN_0_1((distToLight - pointLight.mInnerRadius) / (pointLight.mOuterRadius - pointLight.mInnerRadius));

	pixToLight /= distToLight;
	float diffuseFactor = BETWEEN_0_1(dot(normalize(pixToLight), normal.xyz));
	float4 outputDiffuse = float4(lerp((float3)pointLight.mColor, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
	output += outputDiffuse * diffuseFactor * diffuse;

		//Specular highlights
	float3 reflection = reflect(-pixToLight, normal.xyz);
	float specularMultiple = BETWEEN_0_1(dot(reflection, pixToCam));
	float specFactor = pow(specularMultiple, specular.w);
	float4 outputSpecular = float4(lerp(pointLight.mSpecularColor.xyz, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
	output += outputSpecular * specFactor * float4(specular.xyz, 1.0);
//	output *= (gShadowMap.Sample(linearTextureSampler, -normalize(pixToLight)) * 0.5f);

	return output;
}
