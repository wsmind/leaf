#include "postprocess.h"
#include "tonemapping.h"

Texture2D<float4> radianceTexture: register(t0);
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
    //radiance *= pow(1.0 - length(input.uv - float2(0.5, 0.5)), 2.0);

    // tone mapping (also applies gamma correction)
    float3 color = filmicToneMapping(radiance);

    float luminance = dot(color, float3(0.299, 0.587, 0.114));

    // the FXAA pass requires luminance to be stored in the alpha channel
    output.color = float4(color, luminance);

	return output;
}
