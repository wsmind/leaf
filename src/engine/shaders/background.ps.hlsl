#include "shared.h"

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

PS_OUTPUT main(BACKGROUND_PS_INPUT input)
{
	PS_OUTPUT output;

    float4 projPos = float4(input.worldPosition, 1.0);
    //float3 worldPos = mul(viewProjectionInverseMatrix, projPos).xyz;
    float3 worldPos = mul(viewMatrixInverse, projPos).xyz;
    float diffuse = pow(max(dot(normalize(worldPos), normalize(float3(1.0, 1.0, 1.0))), 0.0), 20.0);
	output.color = float4(lerp(float3(0.8, 0.9, 1.0), float3(1.0, 0.9, 0.8), diffuse), 1.0);

	return output;
}
