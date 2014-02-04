Texture2D gDiffuseTexture;

struct DirectionalLight
{
	float4 mColor;
	float4 mSpecularColor;
	float3 mPosition;
	float mPad;
};

struct PointLight
{
	float4 mColor;
	float4 mSpecularColor;
	float3 mPosition;
	float mInnerRadius, mOuterRadius;
};

cbuffer cbPerFrame : register(b0)
{
	float4 gDirColor;
	float4 gDirSpecColor;
	float3 gDirPos;
	float pad;
	float4 gPointColor;
	float4 gPointSpecularColor;
	float3 gPointPos;
	float  gPointInner, gPointOuter;
	float pad0, pad1, pad2;
	float4 gPointColor1;
	float4 gPointSpecularColor1;
	float3 gPointPos1;
	float  gPointInner1, gPointOuter1;
	float pad3, pad4, pad5;
	float4 gAmbientColor;
	float3 gCamPos;
}

SamplerState DiffuseTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 posWorld : POSITION;
	float2 tex : TEXCOORD0;
};

void CalculatePointLight(PointLight inLight, float3 inNorm, float3 inPos, float3 inPixToCamera, out float4 diffuse, out float4 specular)
{
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 fromPixToLight = inLight.mPosition - inPos;

	float distToLight = length(fromPixToLight);
	float lerpAmount = 0.0f;
	
	if (distToLight > inLight.mOuterRadius)
	{
		return;
	}
	if (distToLight < inLight.mInnerRadius)
	{
		lerpAmount = 1.0f;
	}
	else
	{
		lerpAmount = (distToLight - inLight.mInnerRadius) / inLight.mOuterRadius;
	}

	fromPixToLight /= distToLight;
	float diffuseFactor = dot(normalize(fromPixToLight), inNorm);
	if (diffuseFactor > 0.0f)
	{
		diffuse = float4(lerp((float3)inLight.mColor, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
		diffuse *= diffuseFactor;

		float3 reflection = reflect(normalize(-fromPixToLight), inNorm);
			float specularMultiple = dot(reflection, normalize(inPixToCamera));
		if (specularMultiple > 0)
		{
			float specFactor = pow(specularMultiple, 50.0f);
			specular = inLight.mSpecularColor * specFactor;
		}
	}	
}

float4 main(PixelIn input) : SV_TARGET
{
	input.norm = normalize(input.norm);
	float3 pixToCamera = normalize(gCamPos - input.posWorld);
	float4 finalColor = gDiffuseTexture.Sample(DiffuseTextureSampler, input.tex) * gAmbientColor;
	float NdotL = max(dot(input.norm, normalize(-gDirPos)), 0.0);
	float3 reflection = reflect(normalize(gDirPos), input.norm);
	float dirSpecFactor = pow(max(dot(reflection, pixToCamera), 0.0f), 50.0f);
	finalColor += gDirSpecColor * dirSpecFactor;
	finalColor += gDirColor * NdotL;
	
	
	float4 diffuse, specular;
	PointLight pointLight;
	pointLight.mColor = gPointColor;
	pointLight.mSpecularColor = gPointSpecularColor;
	pointLight.mPosition = gPointPos;
	pointLight.mInnerRadius = gPointInner;
	pointLight.mOuterRadius = gPointOuter;
	CalculatePointLight(pointLight, input.norm, input.posWorld, pixToCamera, diffuse, specular);
	finalColor += diffuse;
	finalColor += specular;
	pointLight.mColor = gPointColor1;
	pointLight.mSpecularColor = gPointSpecularColor1;
	pointLight.mPosition = gPointPos1;
	pointLight.mInnerRadius = gPointInner1;
	pointLight.mOuterRadius = gPointOuter1;
	CalculatePointLight(pointLight, input.norm, input.posWorld, pixToCamera, diffuse, specular);
	finalColor += diffuse;
	finalColor += specular;


	

	return finalColor;
}