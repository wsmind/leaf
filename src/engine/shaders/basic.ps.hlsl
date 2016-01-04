#include "shared.h"

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

PS_OUTPUT main(BASIC_PS_INPUT input)
{
	PS_OUTPUT output;

    const float3 light = normalize(float3(1.0, 1.0, 1.0));

    float3 normal = normalize(input.normal);
    float d = saturate(dot(normal, light));

	output.color = float4(diffuse * d, 1.0);

	return output;
}
