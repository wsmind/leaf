#include "scene.h"
#include "pass.h"
#include "postprocess.h"
#include "bloom.h"

#define SAMPLE_COUNT 9

Texture2D inputTexture: register(t0);
SamplerState inputSampler: register(s0);

Texture2D accumulatorTexture: register(t1);
SamplerState accumulatorSampler: register(s1);

//Texture2D downsampledTexture[8]: register(t1);
//SamplerState downsampledSampler[8]: register(s1);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = inputTexture.Sample(inputSampler, input.uv).rgb;

	/*[unroll]
	for (int i = 0; i < 6; i++)
	{
		color += downsampledTexture[i].Sample(downsampledSampler[i], input.uv).rgb;
	}*/

	float radius = 1.0;

	float2 offsets[SAMPLE_COUNT] = {
		float2(-1.0, -1.0),
		float2(0.0, -1.0),
		float2(1.0, -1.0),
		float2(-1.0, 0.0),
		float2(0.0, 0.0),
		float2(1.0, 0.0),
		float2(-1.0, 1.0),
		float2(0.0, 1.0),
		float2(1.0, 1.0)
	};

	float weights[SAMPLE_COUNT] = {
		1.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0,
		2.0 / 16.0,
		4.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0
	};

	[unroll]
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		color += bloomConstants.intensity * weights[i] * accumulatorTexture.Sample(accumulatorSampler, input.uv + offsets[i] * radius * passConstants.viewportSize.zw).rgb;
	}

	output.color = float4(color, 1.0);

	return output;
}
