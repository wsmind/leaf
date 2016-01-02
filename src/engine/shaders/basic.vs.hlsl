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

    input.pos.xz = rotate(input.pos.xz, time * 0.1);
    input.pos.x *= 9.0 / 16.0;
    input.pos.z += 3.0;
    output.position = float4(input.pos.xy / input.pos.z, input.pos.z / 10.0, 1.0);

	output.normal = input.normal;
    output.uv = input.uv;

	return output;
}
