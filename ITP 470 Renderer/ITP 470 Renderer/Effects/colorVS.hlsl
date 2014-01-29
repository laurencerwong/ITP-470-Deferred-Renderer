cbuffer cbPerFrame
{
	float4x4 gView;
	float4x4 gProj;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
};

struct VertexIn
{
	float3 pos : POSITION;
	float4 color : COLOR;
	float3 norm : NORMAL;
};

struct PixelIn
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

PixelIn main( VertexIn input ) 
{
	PixelIn output;
	output.pos = mul(gWorld, float4(input.pos, 1.0));
	output.pos = mul(gView, output.pos);
	output.pos = mul(gProj, output.pos);
	output.color = input.color;
	return output;
}