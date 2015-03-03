Texture2D gDiffuseTex	: register(t0);
Texture2D gLightAccumTex: register(t1);
Texture2D gNormalTex : register(t2);
Texture2D gPositionTex : register(t3);
Texture2D gDepthTex : register(t4);
Texture2D gRandomTex : register(t5);

#define SAMPLE_RADIUS 0.2f
#define SAMPLE_SCALE 0.1f
#define SAMPLE_BIAS 0.001f	
#define SAMPLE_INTENSITY 0.5f
#define AO_ITERATIONS 4

SamplerState linearTextureSampler
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

cbuffer perFrameCB : register(b0)
{
	float4 gAmbientColor;
};

struct PixelIn
{
	float4 position : SV_POSITION;
	float3 frustCorner : TEXCOORD0;
	float2 tex		: TEXCOORD1;
};

float2 getRandomUV(in float2 uv)
{
	//hard coded screen size: 800x600
	//hard coded random texture size: 64x64
	return normalize(gRandomTex.Sample(linearTextureSampler, float2(800, 600) * uv / float2(64, 64)).xy * 2.0f - 1.0f);
}

float3 getNormal(in float2 uv)
{
	return normalize(gNormalTex.Sample(linearTextureSampler, uv).xyz * 2.0f - 1.0f);
}

float3 getPosition(in float2 uv)
{
	return gPositionTex.Sample(linearTextureSampler, uv).xyz;
}

float calcAmbientOcclusion(in float2 texCoord, in float2 texCoordOffset, in float3 position, in float3 normal)
{
	//gets the vector from the pixel to the occluder
	float3 sampleVector = getPosition(texCoord + texCoordOffset) - position;
		const float3 normSampleVector = normalize(sampleVector);
	const float sampleVectorLenScaled = length(sampleVector) * SAMPLE_SCALE;
	return max(0.0f, dot(normal, normSampleVector) - SAMPLE_BIAS) *	(1.0f / (1.0f + sampleVectorLenScaled)) * SAMPLE_INTENSITY;
}

float getDepth(in float2 uv)
{
	return gDepthTex.Sample(linearTextureSampler, uv).x * 1000.0f;
}

float4 main(PixelIn input) : SV_TARGET
{
	const float2 offsetVecs[4] = { float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1) };
	float3 pixelPos = getPosition(input.tex);
	float3 pixelNorm = getNormal(input.tex);
	float2 rand = getRandomUV(input.tex);
	float aoFactor = 0.0f;
	float aoRadius = SAMPLE_RADIUS / getDepth(input.tex);
	for (int i = 0; i < AO_ITERATIONS; ++i)
	{
		float2 coord1 = reflect(offsetVecs[i], rand) * aoRadius;
			float2 coord2 = float2(coord1.x * 0.707f - coord1.y * 0.707f,
			coord1.x * 0.707f + coord1.y * 0.707f);

		aoFactor += calcAmbientOcclusion(input.tex, coord1*0.25, pixelPos, pixelNorm);
		aoFactor += calcAmbientOcclusion(input.tex, coord2*0.50, pixelPos, pixelNorm);
		aoFactor += calcAmbientOcclusion(input.tex, coord1*0.75, pixelPos, pixelNorm);
		aoFactor += calcAmbientOcclusion(input.tex, coord2     , pixelPos, pixelNorm);
	}
	aoFactor /= (float)AO_ITERATIONS*4.0f;

	float3 diffuse = gDiffuseTex.Sample(linearTextureSampler, input.tex).rgb;
		float4 lightColor = gLightAccumTex.Sample(linearTextureSampler, input.tex);
	//return float4(normalize(gRandomTex.Sample(linearTextureSampler, float2(800, 600) * input.tex / float2(64, 64)).xy), 1.0f, 1.0f);
//		return float4(aoFactor, aoFactor, aoFactor, 1.0f);
	return ((gAmbientColor - aoFactor) * float4(diffuse, 1.0f) + (lightColor * float4(diffuse, 1.0f)));
}