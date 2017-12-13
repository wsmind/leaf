#include "pass.h"
#include "postprocess.h"

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

	// early out if nothing is moving around this pixel
	if (length(maxMotion) < 0.5)
	{
		output.color = float4(centerSample.rgb, 1.0);
		return output;
	}

	float jitter = rand(input.uv) * 2.0 - 1.0;

	const float sampleCount = 20.0;

	float totalWeight = 1.0 / max(length(centerMotion), 0.5);
	float3 totalRadiance = totalWeight * centerSample.rgb;

	for (float i = 0.0; i < sampleCount; i += 1.0)
	{
		float t = lerp(-1.0, 1.0, (i + jitter + 1.0) / (sampleCount + 1.0));

		float2 offset = maxMotion * t;
		float2 currentUv = input.uv + screenToTextureSpace(offset);
		float4 currentSample = radianceTexture.Sample(radianceSampler, currentUv).rgba; // packed color + depth
		float2 currentMotion = motionTexture.Sample(motionSampler, currentUv).rg;

		float foreground = softDepthCompare(centerSample.a, currentSample.a);
		float background = softDepthCompare(currentSample.a, centerSample.a);

		float sampleDistance = length(offset);
		float currentMotionLength = length(currentMotion);

		float weight = 0.0;
		weight += foreground * cone(sampleDistance, currentMotionLength);
		weight += background * cone(sampleDistance, centerMotionLength);
		weight += 2.0 * cylinder(sampleDistance, currentMotionLength) * cylinder(sampleDistance, centerMotionLength);

		totalWeight += weight;
		totalRadiance += weight * currentSample.rgb;
	}

	output.color = float4(totalRadiance / totalWeight, 1.0);
	return output;
}
