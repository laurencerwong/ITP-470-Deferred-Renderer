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
	float2 tex : TEXCOORD0;
};

float4 main(PixelIn input) : SV_TARGET
{
	return gDiffuseTexture.Sample(DiffuseTextureSampler, input.tex);
}