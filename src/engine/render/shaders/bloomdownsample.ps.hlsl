#include "pass.h"
#include "postprocess.h"
#include "scene.h"

Texture2D<float4> inputTexture: register(t0);
SamplerState inputSampler: register(s0);

struct PixelOutput
{
	float4 color: SV_TARGET;
};

float3 fetch4BilinearSamples(float2 uv)
{
	float2 offset = 0.5 * passConstants.viewportSize.zw;

	float3 color = 0.0;
	color += inputTexture.Sample(inputSampler, uv + float2(-offset.x, -offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(-offset.x, offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(offset.x, offset.y)).rgb;
	color += inputTexture.Sample(inputSampler, uv + float2(offset.x, -offset.y)).rgb;

	return color * 0.25;
}

PixelOutput main(POSTPROCESS_PS_INPUT input)
{
	PixelOutput output;

	float3 color = fetch4BilinearSamples(input.uv);
	output.color = float4(color, 1.0);

	return output;
}
