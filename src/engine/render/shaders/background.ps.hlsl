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

    // convert ray to equirectangular coordinates
    float x = (atan2(direction.y, -direction.x) / 3.141592) * 0.5 + 0.5;
    float y = (asin(-direction.z) / 1.570796) * 0.5 + 0.5;

    output.color = environmentTexture.Sample(environmentSampler, float2(x, y));

	return output;
}
