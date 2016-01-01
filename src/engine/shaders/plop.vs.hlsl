#include "shared.h"

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	output.pos = float4(input.pos.xy, 0.0, 1.0);

	input.pos.x *= 16.0 / 9.0;
	output.coord = input.pos.xy;

	return output;
}
