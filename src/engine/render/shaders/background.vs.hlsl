#include "scene.h"
#include "shared.h"

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

BACKGROUND_PS_INPUT main(VS_INPUT input)
{
    BACKGROUND_PS_INPUT output;

    // force z to 1 to make sure the background is always behind everything
	output.pos = float4(input.pos.xy, 1.0, 1.0);
    output.worldPosition = mul(viewProjectionInverseMatrix, output.pos).xyz;

	return output;
}
