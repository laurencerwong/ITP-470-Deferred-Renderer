//#pragma pack_matrix( row_major )

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
	float3 norm : NORMAL;
	float3 tang : TANGENT;
	float3 binorm : BINORMAL;
	float2 tex : TEXCOORD0;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float3 norm : NORMAL;
	float3 posWorld : POSITION;
	float3 tang : TANGENT;
	float3 binorm : BINORMAL;
	float2 tex : TEXCOORD0;
	float  depthWorldView : TEXCOORD1;
	float4 shadowTexCoord : TEXCOORD2;
};


PixelIn main(VertexIn input)
{
	PixelIn output;
	float4x4 worldView = mul(gView, gWorld);
	output.pos = mul(gWorld, float4(input.pos, 1.0));
	output.posWorld = (float3)output.pos;
	output.pos = mul(gView, output.pos);
	output.depthWorldView = output.pos.z; //store world view depth for linear depth buffer
	output.pos = mul(gProj, output.pos);

	output.norm = mul((float3x3)gWorld, input.norm);
	output.tang = mul((float3x3)gWorld, input.tang);
	output.binorm = mul((float3x3)gWorld, input.binorm);
	output.tex = input.tex;
	output.shadowTexCoord = mul(gShadowTransform, float4(input.pos, 1.0f));
	return output;
}