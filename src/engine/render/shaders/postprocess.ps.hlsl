#include "pass.h"
#include "postprocess.h"
#include "tonemapping.h"

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

struct POSTPROCESS_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

float2 offsetUV(float2 uv, float factor)
{
    uv = uv * 2.0 - 1.0;

    float l = length(uv);
    uv *= pow(l, factor);

    uv = uv * 0.5 + 0.5;
    return uv;
}

POSTPROCESS_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    float2 uv = input.uv;

    // pixellate
    if (postProcessConstants.pixellateDivider > 0.0)
    {
        float2 divider = float2(postProcessConstants.pixellateDivider * passConstants.viewportSize.y * passConstants.viewportSize.z, postProcessConstants.pixellateDivider);
        uv = floor(uv / divider) * divider;
    }

    // per-channel offset for chromatic abberation
    float2 uvR = offsetUV(uv, 0.034);
    float2 uvG = offsetUV(uv, 0.024);
    float2 uvB = offsetUV(uv, 0.041);

    float3 radiance = float3(
        radianceTexture.Sample(radianceSampler, uvR).r,
        radianceTexture.Sample(radianceSampler, uvG).g,
        radianceTexture.Sample(radianceSampler, uvB).b
    );

    // vignette
    radiance *= pow(1.0 - length(input.uv - float2(0.5, 0.5)), 1.6);

    // tone mapping (also applies gamma correction)
    float3 color = filmicToneMapping(radiance);

    // the FXAA pass requires luminance to be stored in the alpha channel
    output.color = float4(color, computeLuminance(color));

	return output;
}
