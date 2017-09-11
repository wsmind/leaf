#include "postprocess.h"

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

Texture2D<float4> motionTexture: register(t1);
SamplerState motionSampler: register(s1);

struct MOTIONBLUR_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

MOTIONBLUR_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    MOTIONBLUR_PS_OUTPUT output;

    float3 radiance = radianceTexture.Sample(radianceSampler, input.uv).rgb;
    float2 motion = motionTexture.Sample(motionSampler, input.uv).rg;

    output.color = float4(radiance * float3(motion, 1.0), 1.0);

	return output;
}
