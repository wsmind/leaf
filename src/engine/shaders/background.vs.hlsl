#include "shared.h"

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

BACKGROUND_PS_INPUT main(VS_INPUT input)
{
    BACKGROUND_PS_INPUT output;

	output.pos = float4(input.pos.xy, 1.0, 1.0);
    output.worldPosition = input.pos.xyz; // mul(viewProjectionInverseMatrix, input.pos).xyz;

	return output;
}
