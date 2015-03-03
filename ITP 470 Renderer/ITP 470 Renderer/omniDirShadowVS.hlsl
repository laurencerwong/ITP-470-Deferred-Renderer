cbuffer cbPerFrame : register(cb0)
{
	float4x4 gProj;
	float4x4 gView;
};

cbuffer cbPerObject : register(cb1)
{
	float4x4 gWorld;
	float4x4 gShadowTransform;
};
struct VertexIn
{
	float3 pos : POSITION;
};
struct PS_IN
{
	float4 pos : SV_POSITION;
	float4 posWorld : TEXCOORD0;
};
PS_IN main( VertexIn input )
{
	PS_IN output;
	output.posWorld = mul(gWorld, float4(input.pos, 1.0f));
	output.pos = mul(gView, output.posWorld);
	output.pos = mul(gProj, output.pos);
	return output;
}