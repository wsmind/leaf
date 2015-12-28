#include "shared.h"

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

cbuffer SceneState: register(b0)
{
	float time;
};

float map(float3 pos)
{
	return pos.y + sin(pos.x * 0.5) + sin(pos.z * 0.5);
}

PS_OUTPUT main(PS_INPUT input)
{
	PS_OUTPUT output;

	float3 pos = float3(sin(time) * 2.0, 10.0, 0.0);
	float3 dir = normalize(float3(input.coord, 1.0));
	
	for (int i = 0; i < 64; i++)
	{
		float distance = map(pos);
		pos += dir * distance;
	}

	float3 mtl = frac(pos * 0.1);
	float3 sky = float3(0.8, 0.9, 1.0);

	float fog = 1.0 - exp(-pos.z * 0.01);
	float3 color = lerp(mtl, sky, fog);

	output.color = float4(color, 1.0);

	return output;
}
