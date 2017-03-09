#include "postprocess.h"

struct POSTPROCESS_VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

POSTPROCESS_PS_INPUT main(POSTPROCESS_VS_INPUT input)
{
    POSTPROCESS_PS_INPUT output;
    output.pos = float4(input.pos.xy, 0.0, 1.0);
    output.uv = input.uv;

    return output;
}
