import parameters.scene;
import parameters.pass;
import parameters.postprocess;

#include "tonemapping.h"

struct POSTPROCESS_VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

struct POSTPROCESS_PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 uv: TEXCOORD0;
};

struct POSTPROCESS_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

[shader("vertex")]
POSTPROCESS_PS_INPUT vertexMain(POSTPROCESS_VS_INPUT input)
{
    POSTPROCESS_PS_INPUT output;
    output.pos = float4(input.pos.xy, 0.0, 1.0);
    output.uv = input.uv;

    return output;
}

float2 offsetUV(float2 uv, float factor)
{
    uv = uv * 2.0 - 1.0;

    float l = length(uv) + 1.0;
    uv /= pow(l, factor);

    uv = uv * 0.5 + 0.5;
    return uv;
}

float computeLuminance(float3 color)
{
	return dot(color, float3(0.299, 0.587, 0.114));
}

[shader("fragment")]
POSTPROCESS_PS_OUTPUT fragmentMain(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    float2 uv = input.uv;

    // pixellate
    if (postprocessParameters.constants.pixellateDivider > 0.0)
    {
        float2 divider = float2(postprocessParameters.constants.pixellateDivider * passParameters.constants.viewportSize.y * passParameters.constants.viewportSize.z, postprocessParameters.constants.pixellateDivider);
        uv = floor(uv / divider) * divider;
    }

    float3 colorOffsets = postprocessParameters.constants.abberationStrength * float3(0.34, 0.24, 0.41);

    // per-channel offset for chromatic abberation
    float2 uvR = offsetUV(uv, colorOffsets.r);
    float2 uvG = offsetUV(uv, colorOffsets.g);
    float2 uvB = offsetUV(uv, colorOffsets.b);

    float3 radiance = float3(
        postprocessParameters.radianceTexture.Sample(postprocessParameters.radianceSampler, uvR).r,
        postprocessParameters.radianceTexture.Sample(postprocessParameters.radianceSampler, uvG).g,
        postprocessParameters.radianceTexture.Sample(postprocessParameters.radianceSampler, uvB).b
    );

    // scanline
    radiance *= lerp(1.0, frac((input.uv.y + postprocessParameters.constants.scanlineOffset) * postprocessParameters.constants.scanlineFrequency), postprocessParameters.constants.scanlineStrength);

    // vignette
    radiance *= pow(postprocessParameters.constants.vignetteSize - length(input.uv - float2(0.5, 0.5)), postprocessParameters.constants.vignettePower);

    // tone mapping (also applies gamma correction)
    float3 color = filmicToneMapping(radiance);

    // the FXAA pass requires luminance to be stored in the alpha channel
    output.color = float4(color, computeLuminance(color));

	return output;
}
