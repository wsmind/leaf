#include "shared.h"

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

float2 rotate(float2 v, float a)
{
    float c = cos(a);
    float s = sin(a);
    return mul(float2x2(c, s, -s, c), v);
}

BASIC_PS_INPUT main(VS_INPUT input)
{
	BASIC_PS_INPUT output;

    //input.pos.xy = rotate(input.pos.xy, time * 0.1);
    //input.pos.x *= 9.0 / 16.0;
    //input.pos.z -= 4.0;

    float4 worldPosition = mul(modelMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(viewMatrix, worldPosition);
    output.position = mul(projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.eye = cameraPosition - worldPosition.xyz;
    output.normal = mul((float3x3)modelMatrix, input.normal);
    output.uv = input.uv;

	return output;
}
