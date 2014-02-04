//#pragma pack_matrix( row_major )

cbuffer cbPerFrame : register(cb0)
{
	float4x4 gProj;
	float4x4 gView;

};

cbuffer cbPerObject : register(cb1)
{
	float4x4 gWorld;
};

struct VertexIn
{
	float3 pos : POSITION;
	float3 norm : NORMAL;
	float2 tex : TEXCOORD0;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 posWorld : POSITION;
	float2 tex : TEXCOORD0;
};

PixelIn main(VertexIn input)
{
	PixelIn output;
	output.pos = mul(gWorld, float4(input.pos, 1.0));
	output.posWorld = (float3)output.pos;
	output.pos = mul(gView, output.pos);
	output.pos = mul(gProj, output.pos);

	output.norm = mul((float3x3)gWorld, input.norm);
	output.tex = input.tex;
	return output;
}