#include "postprocess.h"
#include "tonemapping.h"

Texture2DMS<float4> radianceTexture: register(t0);
//SamplerState radianceSampler: register(s0);

struct POSTPROCESS_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

POSTPROCESS_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    float width;
    float height;
    float samples;
    radianceTexture.GetDimensions(width, height, samples);

    // simple MSAA resolve with a box filter, but tone-mapping aware
    float3 color = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < 4; i++)
    {
        float3 radiance = radianceTexture.Load(input.uv * float2(width, height), i).rgb;

        // tone mapping (also applies gamma correction)
        color += reinhardToneMapping(radiance);
    }
    color *= 0.25;

    // vignette
    //radiance *= pow(1.0 - length(input.uv - float2(0.5, 0.5)), 2.0);

    output.color = float4(color, 1.0);

	return output;
}
