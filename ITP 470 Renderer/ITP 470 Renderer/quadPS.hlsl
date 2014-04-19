Texture2D gDiffuseTexture : register(t0);

SamplerState DiffuseTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 frustCorner : TEXCOORD0;
	float2 tex : TEXCOORD1;
};

float4 main(PixelIn input) : SV_TARGET
{
	//return (2.0 * 0.01f) / (3.0f + 0.01f - gDiffuseTexture.Sample(DiffuseTextureSampler, input.tex) * (3.0f - 0.01f));
	return gDiffuseTexture.Sample(DiffuseTextureSampler, input.tex);
}