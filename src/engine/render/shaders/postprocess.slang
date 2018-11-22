#include "pass.h"
#include "tonemapping.h"

#include "constants/PostProcessConstants.h"

cbuffer PostProcessConstants : register(b2)
{
    PostProcessConstants postProcessConstants;
};

struct POSTPROCESS_PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 uv: TEXCOORD0;
};

float computeLuminance(float3 color)
{
	return dot(color, float3(0.299, 0.587, 0.114));
}

struct POSTPROCESS_VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

[shader("vertex")]
POSTPROCESS_PS_INPUT vertexMain(POSTPROCESS_VS_INPUT input)
{
    POSTPROCESS_PS_INPUT output;
    output.pos = float4(input.pos.xy, 0.0, 1.0);
    output.uv = input.uv;

    return output;
}

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

struct POSTPROCESS_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

float2 offsetUV(float2 uv, float factor)
{
    uv = uv * 2.0 - 1.0;

    float l = length(uv) + 1.0;
    uv /= pow(l, factor);

    uv = uv * 0.5 + 0.5;
    return uv;
}

[shader("fragment")]
POSTPROCESS_PS_OUTPUT fragmentMain(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    float2 uv = input.uv;

    // pixellate
    if (postProcessConstants.pixellateDivider > 0.0)
    {
        float2 divider = float2(postProcessConstants.pixellateDivider * passConstants.viewportSize.y * passConstants.viewportSize.z, postProcessConstants.pixellateDivider);
        uv = floor(uv / divider) * divider;
    }

    float3 colorOffsets = postProcessConstants.abberationStrength * float3(0.34, 0.24, 0.41);

    // per-channel offset for chromatic abberation
    float2 uvR = offsetUV(uv, colorOffsets.r);
    float2 uvG = offsetUV(uv, colorOffsets.g);
    float2 uvB = offsetUV(uv, colorOffsets.b);

    float3 radiance = float3(
        radianceTexture.Sample(radianceSampler, uvR).r,
        radianceTexture.Sample(radianceSampler, uvG).g,
        radianceTexture.Sample(radianceSampler, uvB).b
    );

    // scanline
    radiance *= lerp(1.0, frac((input.uv.y + postProcessConstants.scanlineOffset) * postProcessConstants.scanlineFrequency), postProcessConstants.scanlineStrength);

    // vignette
    radiance *= pow(postProcessConstants.vignetteSize - length(input.uv - float2(0.5, 0.5)), postProcessConstants.vignettePower);

    // tone mapping (also applies gamma correction)
    float3 color = filmicToneMapping(radiance);

    // the FXAA pass requires luminance to be stored in the alpha channel
    output.color = float4(color, computeLuminance(color));

	return output;
}
