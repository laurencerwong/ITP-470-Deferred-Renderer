cbuffer cbPerFrame : register(cb0)
{
	float4x4 gProj;
	float4x4 gView;
	float3 gFarFrustrumCorners[4];
};

cbuffer cbPerObject : register(cb1)
{
	float4x4 gWorld;
};

struct VertexIn
{
	float3 pos : POSITION;
	float3 tex : TEXCOORD0;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float3 frustCorner : TEXCOORD0;
	float2 tex : TEXCOORD1;
};


VertexOut main( VertexIn input )
{
	VertexOut output;
	output.pos = float4(input.pos, 1.0f);
	output.tex = input.tex.xy;
	output.frustCorner = gFarFrustrumCorners[input.tex.z];
	return output;
}