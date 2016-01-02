#include "shared.h"

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

PS_OUTPUT main(BASIC_PS_INPUT input)
{
	PS_OUTPUT output;

    float3 normal = normalize(input.normal);
	output.color = float4(normal, 1.0);

	return output;
}
