#include "postprocess.h"

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

Texture2D<float4> motionTexture: register(t1);
SamplerState motionSampler: register(s1);

Texture2D<float4> tileMaxTexture: register(t2);

SamplerState PointSampler
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

struct MOTIONBLUR_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

MOTIONBLUR_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    MOTIONBLUR_PS_OUTPUT output;

    float3 radiance = radianceTexture.Sample(radianceSampler, input.uv).rgb;
    //float2 motion = motionTexture.Sample(motionSampler, input.uv).rg;
    float2 motion = tileMaxTexture.Sample(PointSampler, input.uv).rg;

    for (float n = 0.1f; n <= 1.0f; n += 0.1f)
    {
        radiance += radianceTexture.Sample(radianceSampler, saturate(input.uv + motion * n)).rgb;
        radiance += radianceTexture.Sample(radianceSampler, saturate(input.uv - motion * n)).rgb;
    }

    output.color = float4(radiance / 21.0f, 1.0);
    //output.color = float4(saturate(motion), 0.0, 1.0);

	return output;
}
