import streams.geometry2d;

import parameters.scene;
import parameters.pass;
import parameters.postprocess;

import tonemapping;
import color;

struct VertexOutput
{
	float4 position: SV_POSITION;
	float2 uv: TEXCOORD0;
};

struct PixelOutput
{
	float4 color: SV_TARGET;
};

[shader("vertex")]
VertexOutput vertexMain(Geometry2D geometry)
{
    VertexOutput output;
    output.position = float4(geometry.vertex.position.xy, 0.0, 1.0);
    output.uv = geometry.vertex.uv;

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

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
    PixelOutput output;

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
