#include "shared.h"

struct VS_INPUT
{
	float2 pos: POSITION;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;

	output.pos = float4(input.pos, 0.0, 1.0);

	input.pos.x *= 16.0 / 9.0;
	output.coord = input.pos;

	return output;
}
