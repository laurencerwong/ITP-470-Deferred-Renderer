Texture2D gDiffuseTex	: register(t0);
Texture2D gNormalTex	: register(t1);
Texture2D gShadowMap	: register(t2);
Texture2D gZPrepassTex	: register(t3);

SamplerState linearTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerComparisonState ShadowMapSampler
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS_EQUAL;
};

struct Material
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;
};

cbuffer cbPerFrame : register(b0)
{
	float4 gFarPlane;
}

cbuffer cbPerObject : register(b1)
{
	Material gMaterial;
}

struct PS_OUT
{
	float4 diffuse	: SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 specular	: SV_TARGET2;
	float4 position : SV_TARGET3;
	float depth : SV_TARGET4;
};

struct PS_IN
{
	float4 position	: SV_POSITION;
	float3 normal	: NORMAL;
	float3 posWorld : POSITION;
	float3 tangent	: TANGENT;
	float3 binormal	: BINORMAL;
	float2 tex	: TEXCOORD0; 
	float  depthWorldView : TEXCOORD1;
	float4 shadowTexCoord : TEXCOORD2;
};

PS_OUT main(PS_IN input) 
{
	PS_OUT output = (PS_OUT)0;
	if (input.depthWorldView <= gZPrepassTex.Sample(linearTextureSampler, input.position.xy).r * 1000.0f)
	{ 
		output.diffuse = gDiffuseTex.Sample(linearTextureSampler, input.tex);

		//normal map calculations
		float3 transTangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
			float3 bitangent = normalize(input.binormal);
			float3x3 NTB = float3x3(transTangent, bitangent, input.normal);
			output.normal.xyz = gNormalTex.Sample(linearTextureSampler, input.tex).rgb;
		output.normal = 2.0f * output.normal - 1.0f;
		output.normal.xyz = normalize(mul(output.normal.xyz, NTB));

		output.specular = gMaterial.mSpecular;
		output.position = float4(input.posWorld, 1.0f);

		output.depth = input.depthWorldView / gFarPlane.x;

		float depth = input.shadowTexCoord.z - 0.001;
		float shadowFactor = gShadowMap.SampleCmpLevelZero(ShadowMapSampler, input.shadowTexCoord.xy, depth).r;
		output.normal.w = shadowFactor; // storing whether or not the pixel is in shadow in the normal.w
		//because I couldn't think of anywhere else to put it

	}
	return output;
}