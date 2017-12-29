#include "pass.h"
#include "postprocess.h"
#include "scene.h"

Texture2D<float4> inputTexture: register(t0);
SamplerState inputSampler: register(s0);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = inputTexture.Sample(inputSampler, input.uv + passConstants.viewportSize.zw * 0.25).rgb;
	float luminance = computeLuminance(color);

	float threshold = 1.0;
	color *= smoothstep(0.0, threshold, luminance);

	output.color = float4(color, 1.0);

	return output;
}
