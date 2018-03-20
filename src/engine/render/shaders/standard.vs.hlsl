#include "pass.h"
#include "scene.h"
#include "standard.h"

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
	float4x4 modelMatrix: MODELMATRIX;
	float4x4 worldToPreviousFrameClipSpaceMatrix: WORLDTOPREVIOUSFRAMECLIPSPACE;
	float3x3 normalMatrix: NORMALMATRIX;
};

STANDARD_PS_INPUT main(VS_INPUT input)
{
    STANDARD_PS_INPUT output;

    float4 worldPosition = mul(input.modelMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(passConstants.viewMatrix, worldPosition);
    output.position = mul(passConstants.projectionMatrix, viewPosition);
   
    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.viewPosition = viewPosition.xyz;
    output.marchingStep = (output.worldPosition - passConstants.cameraPosition) / MARCHING_ITERATIONS;
    output.normal = mul(input.normalMatrix, input.normal);
    output.tangent = float4(mul(input.normalMatrix, input.tangent.xyz), input.tangent.w);
    output.uv = float2(input.uv.x, 1.0 - input.uv.y);
    output.clipPosition = output.position;

	output.worldToPreviousFrameClipSpaceMatrix = input.worldToPreviousFrameClipSpaceMatrix;

    return output;
}
