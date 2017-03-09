#include "instance.h"
#include "scene.h"
#include "standard.h"

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

STANDARD_PS_INPUT main(VS_INPUT input)
{
    STANDARD_PS_INPUT output;

    float4 worldPosition = mul(modelMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(viewMatrix, worldPosition);
    output.position = mul(projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.normal = mul((float3x3)modelMatrix, input.normal);
    output.tangent = float4(mul((float3x3)modelMatrix, input.tangent.xyz), input.tangent.w);
    output.uv = float2(input.uv.x, 1.0 - input.uv.y);

    return output;
}
