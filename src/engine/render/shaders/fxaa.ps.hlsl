#include "scene.h"
#include "pass.h"
#include "postprocess.h"

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12
#include "Fxaa3_11.h"

Texture2D<float4> image: register(t0);
SamplerState samplerState: register(s0);

POSTPROCESS_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    POSTPROCESS_PS_OUTPUT output;

    FxaaTex tex;
    tex.smpl = samplerState;
    tex.tex = image;

    output.color = FxaaPixelShader(
        input.uv,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),
        tex,
        tex,
        tex,
        passConstants.viewportSize.zw, // = 1.0 / resolution
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),
        0.75,
        0.166,
        0.0833,
        0.0,
        0.0,
        0.0,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0)
    );

	return output;
}
