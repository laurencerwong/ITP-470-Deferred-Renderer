cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
};

struct VertexIn
{
	float3 pos : POSITION;
	float4 color : COLOR;
};

struct VertexOut
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

VertexOut main( VertexIn input ) 
{
	VertexOut output;
	output.pos = mul(float4(input.pos, 1.0), gWorldViewProj);
	output.color = input.color;
	return output;
}