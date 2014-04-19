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
	float3 position : POSITION;
};

struct PixelIn
{
	float4 position : SV_POSITION;
};

PixelIn main(VertexIn input)
{
	PixelIn output;
	output.position = mul(gWorld, float4(input.position, 1.0));
	output.position = mul(gView, output.position);
	output.position = mul(gProj, output.position);

	return output;
}
