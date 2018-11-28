import streams.geometry2d;

import parameters.scene;
import parameters.pass;
import parameters.postprocess;

import color;

#define FXAA_PC 1
#define FXAA_HLSL_5 1
#define FXAA_QUALITY__PRESET 12
#include "Fxaa3_11.h"

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

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
    PixelOutput output;

    FxaaTex tex;
    tex.smpl = postprocessParameters.radianceSampler;
    tex.tex = postprocessParameters.radianceTexture;

    output.color = FxaaPixelShader(
        input.uv,
        FxaaFloat4(0.0, 0.0, 0.0, 0.0),
        tex,
        tex,
        tex,
        passParameters.constants.viewportSize.zw, // = 1.0 / resolution
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
