#include "equirectangular.h"
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
    float2 uv = directionToEquirectangularUV(direction);
    output.color = environmentTexture.Sample(environmentSampler, uv);

	return output;
}
