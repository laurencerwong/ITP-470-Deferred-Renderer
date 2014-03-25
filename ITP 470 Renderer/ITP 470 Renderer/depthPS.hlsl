struct PixelIn
{
	float4 pos : SV_POSITION;
	float4 depthPos : TEXTURE0;
};

float4 main(PixelIn input) : SV_TARGET
{
	float depth = input.depthPos.z / input.depthPos.w;
	return float4(depth, depth, depth, 1.0f);
}