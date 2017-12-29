#include "pass.h"
#include "postprocess.h"
#include "scene.h"

Texture2D inputTexture: register(t0);
SamplerState inputSampler: register(s0);

Texture2D downsampledTexture[4]: register(t1);
SamplerState downsampledSampler[4]: register(s1);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = inputTexture.Sample(inputSampler, input.uv).rgb;

	color += downsampledTexture[0].Sample(downsampledSampler[0], input.uv).rgb;
	color += downsampledTexture[1].Sample(downsampledSampler[1], input.uv).rgb;
	color += downsampledTexture[2].Sample(downsampledSampler[2], input.uv).rgb;
	color += downsampledTexture[3].Sample(downsampledSampler[3], input.uv).rgb;

	output.color = float4(color, 1.0);

	return output;
}
