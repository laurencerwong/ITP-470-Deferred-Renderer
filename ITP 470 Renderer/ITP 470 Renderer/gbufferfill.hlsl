Texture2D gDiffuseTex	: register(t0);
Texture2D gNormalTex	: register(t1);

SamplerState linearTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct Material
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;
}; 

cbuffer cbPerObject : register(b1)
{
	Material gMaterial;
}

struct PS_OUT
{
	float4 diffuse	: SV_TARGET0;
	float3 normal	: SV_TARGET1;
	float4 specular	: SV_TARGET2;
	float4 position : SV_TARGET3;
};

struct PS_IN
{
	float4 position	: SV_POSITION;
	float3 normal	: NORMAL;
	float3 posWorld : POSITION;
	float3 tangent	: TANGENT;
	float3 binormal	: BINORMAL;
	float2 tex	: TEXCOORD0; 
};

PS_OUT main(PS_IN input) 
{
	PS_OUT output;
	output.diffuse = gDiffuseTex.Sample(linearTextureSampler, input.tex);
	
	//normal map calculations
	float3 transTangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(input.binormal);
	float3x3 NTB = float3x3(transTangent, bitangent, input.normal);
	output.normal = gNormalTex.Sample(linearTextureSampler, input.tex).rgb;
	output.normal = 2.0f * output.normal - 1.0f;
	output.normal = normalize(mul(output.normal, NTB));

	output.specular = gMaterial.mSpecular;
	output.position = float4(input.posWorld, 1.0f);
	return output;
}