#include "pass.h"
#include "scene.h"
#include "unlit.h"

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

UNLIT_PS_INPUT main(VS_INPUT input)
{
    UNLIT_PS_INPUT output;

    float2 uv = input.uv * unlitConstants.uvScale + unlitConstants.uvOffset;

    float4 worldPosition = mul(input.modelMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(passConstants.viewMatrix, worldPosition);
    output.position = mul(passConstants.projectionMatrix, viewPosition);
   
    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.viewPosition = viewPosition.xyz;
    output.uv = float2(uv.x, 1.0 - uv.y);
    output.clipPosition = output.position;

	output.worldToPreviousFrameClipSpaceMatrix = input.worldToPreviousFrameClipSpaceMatrix;

    return output;
}
