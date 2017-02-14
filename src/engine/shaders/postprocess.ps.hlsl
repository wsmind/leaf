#include "postprocess.h"

Texture2D radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

struct POSTPROCESS_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

POSTPROCESS_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    float3 radiance = radianceTexture.Sample(radianceSampler, input.uv).rgb;

    // vignette
    radiance *= pow(1.0 - length(input.uv - float2(0.5, 0.5)), 2.0);

    // gamma correction
    float3 color = pow(radiance, 1.0 / 2.2);

    output.color = float4(color, 1.0);

	return output;
}
