#include "shared.h"

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

BASIC_PS_INPUT main(VS_INPUT input)
{
	BASIC_PS_INPUT output;

    output.position = float4(input.pos.xy, 1.0, 1.0);
    output.uv = input.pos.xy * 0.5 + 0.5;
    output.uv.y = 1.0 - output.uv.y;

	return output;
}
