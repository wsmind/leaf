#include "pass.h"
#include "postprocess.h"
#include "scene.h"

Texture2D inputTexture: register(t0);
SamplerState inputSampler: register(s0);

Texture2D downsampledTexture[8]: register(t1);
SamplerState downsampledSampler[8]: register(s1);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = inputTexture.Sample(inputSampler, input.uv).rgb;

	[unroll]
	for (int i = 0; i < 6; i++)
	{
		color += downsampledTexture[i].Sample(downsampledSampler[i], input.uv).rgb;
	}

	//color = downsampledTexture[7].Sample(downsampledSampler[7], input.uv).rgb;
	output.color = float4(color, 1.0);

	return output;
}
