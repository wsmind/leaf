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
   
    float4 previousFramePosition = mul(previousFrameViewProjectionMatrix, mul(previousFrameModelMatrix, float4(input.pos, 1.0)));
    float2 frameMovement = (output.position.xy / output.position.w) - (previousFramePosition.xy / previousFramePosition.w);
    output.motion = 0.5 * frameMovement * motionSpeedFactor;

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.viewPosition = viewPosition.xyz;
    output.marchingStep = (output.worldPosition - cameraPosition) / MARCHING_ITERATIONS;
    output.normal = mul(normalMatrix, input.normal);
    output.tangent = float4(mul(normalMatrix, input.tangent.xyz), input.tangent.w);
    output.uv = float2(input.uv.x, 1.0 - input.uv.y);

    return output;
}
