#include "depthonly.h"

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

cbuffer DepthOnlyConstants : register(b0)
{
    float4x4 transformMatrix;
};

DEPTHONLY_PS_INPUT main(VS_INPUT input)
{
    DEPTHONLY_PS_INPUT output;

    output.position = mul(transformMatrix, float4(input.pos, 1.0));

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}
