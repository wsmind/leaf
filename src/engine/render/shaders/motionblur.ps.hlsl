#include "pass.h"
#include "postprocess.h"
#include "scene.h"

// sources:
// http://casual-effects.com/research/McGuire2012Blur/McGuire12Blur.pdf
// http://research.nvidia.com/sites/default/files/pubs/2013-11_A-Fast-and/Guertin2013MotionBlur-small.pdf

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler: register(s0);

Texture2D<float4> motionTexture: register(t1);
SamplerState motionSampler: register(s1);

Texture2D<float4> neighborMaxTexture: register(t2);
SamplerState neighborMaxSampler: register(s2);

struct MOTIONBLUR_PS_OUTPUT
{
	float4 color: SV_TARGET;
};

float rand(float2 uv)
{
	return frac(sin(dot(uv.xy, float2(12.9898, 78.233)) * 43758.5453));
}

float2 screenToTextureSpace(float2 direction)
{
	return passConstants.viewportSize.zw * float2(direction.x, -direction.y);
}

float softDepthCompare(float z1, float z2)
{
	return saturate(1.0 - (z1 - z2) / min(z1, z2));
}

float cone(float distance, float speed)
{
	return saturate(1.0 - distance / speed);
}

float cylinder(float distance, float speed)
{
	return 1.0 - smoothstep(0.95 * speed, 1.05 * speed, distance);
}

MOTIONBLUR_PS_OUTPUT main(POSTPROCESS_PS_INPUT input)
{
    MOTIONBLUR_PS_OUTPUT output;

	float4 centerSample = radianceTexture.Sample(radianceSampler, input.uv).rgba; // packed color + depth
	float2 centerMotion = motionTexture.Sample(motionSampler, input.uv).rg;
	float centerMotionLength = length(centerMotion);

	float2 maxMotion = neighborMaxTexture.Sample(neighborMaxSampler, input.uv).rg;
	float maxMotionLength = length(maxMotion);
	float2 maxMotionDirection = normalize(maxMotion);

	// early out if nothing is moving around this pixel
	if (maxMotionLength < 0.5)
	{
		output.color = float4(centerSample.rgb, 1.0);
		return output;
	}

	float totalWeight = 1.0;
	float3 totalRadiance = totalWeight * centerSample.rgb;

	float alternator = 0.0f;
	for (float t = -1.0; t <= 1.0; t += 0.1)
	{
		float2 offset = lerp(maxMotion, centerMotion, alternator) * t;
		alternator = 1.0 - alternator;

		float2 currentUv = input.uv + screenToTextureSpace(offset);
		float4 currentSample = radianceTexture.SampleLevel(radianceSampler, currentUv, 0).rgba; // packed color + depth
		float2 currentMotion = motionTexture.SampleLevel(motionSampler, currentUv, 0).rg;
		float currentMotionLength = length(currentMotion);

		float sampleDistance = length(offset);

		float weight = max(abs(dot(normalize(currentMotion), normalize(centerMotion))), step(0, currentMotionLength - sampleDistance));

		totalWeight += weight;
		totalRadiance += weight * currentSample.rgb;
	}

	output.color = float4(totalRadiance / totalWeight, 1.0);
	return output;
}
