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

struct DirectionalLight
{
	float4 mColor;
	float4 mSpecularColor;
	float3 mPosition;
	float mPad;
};

cbuffer perFrameCB : register(b0)
{
	float4x4 gShadowTransform;
	float4 gAmbientColor;
	float3 gCamPos;
};

cbuffer perLightCB : register(b1)
{
	DirectionalLight directionalLight;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 frustCorner : TEXCOORD0;
	float2 tex : TEXCOORD1;
};


float4 main(PixelIn input) : SV_TARGET
{
	float4 diffuse = gDiffuseTex.Sample(linearTextureSampler, input.tex);
	float4 normal = normalize(gNormalTex.Sample(linearTextureSampler, input.tex));
	float4 specular = gSpecularTex.Sample(linearTextureSampler, input.tex);
	float4 position = gPositionTex.Sample(linearTextureSampler, input.tex);

	float3 pixToCam = normalize(gCamPos - position.xyz);
	float NdotL = max(dot(normal.xyz, normalize(-directionalLight.mPosition)), 0.0);
	float4 output = float4(0.0, 0.0, 0.0, 0.0);

	output += directionalLight.mColor * NdotL;
	float3 reflection = reflect(normalize(directionalLight.mPosition), normal.xyz);
	float specularMultiple = BETWEEN_0_1(dot(reflection, pixToCam));
	float specFactor = pow(specularMultiple, specular.w);
	output += directionalLight.mSpecularColor * specFactor * float4(specular.xyz, 1.0);
	output *= normal.w;
	return output;
}