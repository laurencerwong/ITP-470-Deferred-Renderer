Texture2D gDiffuseTexture : register(t0);
Texture2D gNormalTexture : register(t1);

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

struct Material
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;
};

cbuffer cbPerFrame : register(b0)
{
	DirectionalLight gDirLight;
	PointLight pointLight[4];
	float4 gAmbientColor;
	float3 gCamPos;
}

cbuffer cbPerObject : register(b1)
{
	Material gMaterial;
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
	float3 tang : TANGENT;
	float3 binorm : BINORMAL;
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
		lerpAmount = (distToLight - inLight.mInnerRadius) / (inLight.mOuterRadius - inLight.mInnerRadius);
	}

	fromPixToLight /= distToLight;
	float diffuseFactor = dot(normalize(fromPixToLight), inNorm);
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		diffuse = float4(lerp((float3)inLight.mColor, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
		diffuse *= diffuseFactor * gMaterial.mDiffuse;

		float3 reflection = reflect(-fromPixToLight, inNorm);
		float specularMultiple = dot(reflection, normalize(inPixToCamera));
		[flatten]
		if (specularMultiple > 0)
		{
			float specFactor = pow(specularMultiple, gMaterial.mSpecular.w);
			specular = float4(lerp((float3)inLight.mSpecularColor, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
			specular *= specFactor * gMaterial.mSpecular;
		}
	}	
}

float4 main(PixelIn input) : SV_TARGET
{
//	float depth = 1.0f - input.pos.z / input.pos.w;
	//return float4(depth, depth, depth, 1.0f);


	input.norm = normalize(input.norm);
	//calculate NTB
	float3 transformedTang = normalize(input.tang - dot(input.tang, input.norm) * input.norm);
		float3 bitang = normalize(input.binorm);
	float3x3 NTB = float3x3(transformedTang, bitang, input.norm);

		float3 pixToCamera = normalize(gCamPos - input.posWorld);
		float4 texColor = gDiffuseTexture.Sample(DiffuseTextureSampler, input.tex);
		float3 normSample = gNormalTexture.Sample(DiffuseTextureSampler, input.tex).rgb;
		normSample = 2.0f*normSample - 1.0f;

	input.norm = normalize(mul(normSample,  NTB));

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ambient = gAmbientColor * gMaterial.mAmbient;
	if (gDirLight.mColor.a > 0)
	{
		float NdotL = max(dot(input.norm, normalize(-gDirLight.mPosition)), 0.0);
		if (NdotL > 0.0f)
		{
			float3 reflection = reflect(normalize(gDirLight.mPosition), input.norm);
				float dirSpecFactor = pow(max(dot(reflection, pixToCamera), 0.0f), gMaterial.mSpecular.w);
			diffuse = gDirLight.mColor * NdotL *gMaterial.mDiffuse;
			specular = saturate(gDirLight.mSpecularColor * dirSpecFactor * gMaterial.mSpecular);
		}
	}

	float4 finalColor = texColor * (ambient + diffuse) + specular;
	[unroll]
	for (int i = 0; i < 4; ++i)
	{
		CalculatePointLight(pointLight[i], input.norm, input.posWorld, pixToCamera, diffuse, specular);
		finalColor += diffuse;
		finalColor += specular;
	}
	
	finalColor.w = 1.0f;
	return finalColor;
}