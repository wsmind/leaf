#include "depthonly.h"
#include "pass.h"

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
	float4x4 transformMatrix: TRANSFORM;
};

DEPTHONLY_PS_INPUT main(VS_INPUT input)
{
    DEPTHONLY_PS_INPUT output;

    float4 worldPosition = mul(input.transformMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(passConstants.viewMatrix, worldPosition);
    output.position = mul(passConstants.projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}
