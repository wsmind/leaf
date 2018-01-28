#include "shared.h"

Texture2D<float4> environmentTexture: register(t0);
SamplerState environmentSampler: register(s0);

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

PS_OUTPUT main(BACKGROUND_PS_INPUT input)
{
	PS_OUTPUT output;

    float3 direction = normalize(input.worldPosition);
    float3 sky = (direction.z * 0.5 + 0.5) * float3(0.8, 0.9, 1.0);
    float sun = pow(saturate(dot(direction, normalize(float3(1.0, 1.0, 1.0))) + 0.005), 100.0);
	output.color = float4(lerp(sky, float3(1.0, 0.9, 0.8), sun), 1.0);

    output.color = environmentTexture.Sample(environmentSampler, direction.xy);

	return output;
}
