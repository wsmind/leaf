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

	float3 color = inputTexture.Sample(inputSampler, input.uv).rgb;

	output.color = float4(color, 1.0);

	return output;
}
