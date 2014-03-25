cbuffer cbPerFrame
{
	float4x4 gProj;
	float4x4 gView;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gShadowTransform;
};

struct VertexIn
{
	float3 pos : POSITION;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 depthPos : TEXTURE0;
};

VertexOut main( VertexIn input )
{
	VertexOut output;
	output.pos = mul(gWorld, float4(input.pos, 1.0f));
	output.pos = mul(gView, output.pos);
	output.pos = mul(gProj, output.pos);


	output.depthPos = output.pos;

	return output;
}