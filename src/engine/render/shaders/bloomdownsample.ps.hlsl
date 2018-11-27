#include "scene.h"
#include "pass.h"
#include "postprocess.h"
#include "bloom.h"

#define SAMPLE_COUNT 13

Texture2D<float4> inputTexture: register(t0);
SamplerState inputSampler: register(s0);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

/*float3 fetch4BilinearSamples(float2 uv)
{
	float2 offset = 0.5 * passConstants.viewportSize.zw;

	float3 color = 0.0;
	color += inputTexture.Sample(inputSampler, uv + float2(-offset.x, -offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(-offset.x, offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(offset.x, offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(offset.x, -offset.y)).rgb;

	return color * 0.25;
}*/

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = 0.0;

	float2 offsets[SAMPLE_COUNT] = {
		float2(-0.5, -0.5),
		float2(-0.5, 0.5),
		float2(0.5, 0.5),
		float2(0.5, -0.5),
		float2(-1.0, -1.0),
		float2(-1.0, 1.0),
		float2(1.0, 1.0),
		float2(1.0, -1.0),
		float2(-1.0, 0.0),
		float2(1.0, 0.0),
		float2(0.0, -1.0),
		float2(0.0, 1.0),
		float2(0.0, 0.0)
	};

	float weights[SAMPLE_COUNT] = {
		0.5 / 4.0,
		0.5 / 4.0,
		0.5 / 4.0,
		0.5 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125
	};

	[unroll]
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		color += weights[i] * inputTexture.Sample(inputSampler, input.uv + offsets[i] * passConstants.viewportSize.zw).rgb;
	}

	output.color = float4(color, 1.0);

	return output;
}
