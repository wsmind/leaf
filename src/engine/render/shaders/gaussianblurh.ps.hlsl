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

	// use bilinear filtering hardware to compute a 5-tap gaussian blur with only 3 weighted samples
	float offsets[3] = { -1.2, 0.0, 1.2 };
	float weights[3] = { 5.0 / 16.0, 6.0 / 16.0, 5.0 / 16.0 };

	float3 color = float3(0.0, 0.0, 0.0);

	[unroll]
	for (int i = 0; i < 3; i++)
	{
		color += weights[i] * inputTexture.Sample(inputSampler, input.uv + float2(offsets[i] * passConstants.viewportSize.z, 0.0)).rgb;
	}

	output.color = float4(color, 1.0);

	return output;
}
