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
	float2 tex : TEXCOORD0;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};


VertexOut main( VertexIn input )
{
	VertexOut output;
	output.pos = float4(input.pos, 1.0f);
	output.tex = input.tex;
	return output;
}