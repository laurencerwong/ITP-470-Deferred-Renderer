Texture2D gDiffuseTex	: register(t0);
Texture2D gNormalTex	: register(t1);
Texture2D gSpecularTex	: register(t2);
Texture2D gPositionTex	: register(t3);
Texture2D gDepthTex		: register(t4);

#define BETWEEN_0_1(num) (min(max((num), 0.0), 1.0))

SamplerState linearTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
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

struct PixelIn
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PixelIn input) : SV_TARGET
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
	float4 output = gAmbientColor * pointLight.mColor;
	lerpAmount = BETWEEN_0_1((distToLight - pointLight.mInnerRadius) / (pointLight.mOuterRadius - pointLight.mInnerRadius));

	pixToLight /= distToLight;
	float diffuseFactor = BETWEEN_0_1(dot(normalize(pixToLight), normal.xyz));
	float4 outputDiffuse = float4(lerp((float3)pointLight.mColor, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
	output += outputDiffuse * diffuseFactor * diffuse;
	float3 reflection = reflect(-pixToLight, normal.xyz);
	float specularMultiple = BETWEEN_0_1(dot(reflection, pixToCam));
	float specFactor = pow(specularMultiple, specular.w);
	float4 outputSpecular = float4(lerp(pointLight.mSpecularColor.xyz, float3(0.0f, 0.0f, 0.0f), lerpAmount), 1.0f);
	output += outputSpecular * specFactor * float4(specular.xyz, 1.0);
	return output;
}