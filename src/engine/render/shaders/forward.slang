#include "pass.h"
#include "scene.h"
#include "shadows.h"

import bsdf;
import environment;

#include "constants/ShadowConstants.h"

cbuffer ShadowConstants : register(b2)
{
    ShadowConstants shadowConstants;
};

Texture2D shadowMap: register(t0);
SamplerState shadowMapSampler: register(s0);

Texture2D environmentMap: register(t1);
SamplerState environmentSampler : register(s1);

struct FORWARD_PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 viewPosition : POSITION1;
    float3 marchingStep : POSITION2;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD0;
    float4 clipPosition : TEXCOORD1;
	float4x4 worldToPreviousFrameClipSpaceMatrix : TEXCOORD2;
};

struct FORWARD_PS_OUTPUT
{
    float4 radiance : SV_TARGET0;
    float4 motion : SV_TARGET1;
};

#define MARCHING_ITERATIONS 64

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
	float4x4 modelMatrix: MODELMATRIX;
	float4x4 worldToPreviousFrameClipSpaceMatrix: WORLDTOPREVIOUSFRAMECLIPSPACE;
	float3x3 normalMatrix: NORMALMATRIX;
};

[shader("vertex")]
FORWARD_PS_INPUT vertexMain(VS_INPUT input)
{
    FORWARD_PS_INPUT output;

    float2 uv = input.uv;

    float4 worldPosition = mul(input.modelMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(passConstants.viewMatrix, worldPosition);
    output.position = mul(passConstants.projectionMatrix, viewPosition);
   
    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.viewPosition = viewPosition.xyz;
    output.marchingStep = (output.worldPosition - passConstants.cameraPosition) / MARCHING_ITERATIONS;
    output.normal = mul(input.normalMatrix, input.normal);
    output.tangent = float4(mul(input.normalMatrix, input.tangent.xyz), input.tangent.w);
    output.uv = float2(uv.x, 1.0 - uv.y);
    output.clipPosition = output.position;

	output.worldToPreviousFrameClipSpaceMatrix = input.worldToPreviousFrameClipSpaceMatrix;

    return output;
}

float computeLightFalloff(float distance, float radius)
{
    // see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
    float numerator = saturate(1.0 - pow(distance / radius, 4.0));
    numerator *= numerator;
    return numerator / (distance * distance + 1.0);
}

float sampleShadowMap(int index, float3 worldPosition)
{
    float4 shadowCoords = mul(shadowConstants.lightMatrix[index], float4(worldPosition, 1.0));
    shadowCoords.z = (shadowCoords.z + shadowCoords.w) * 0.5; // hack; GL to DX clip space
    shadowCoords /= shadowCoords.w;
	shadowCoords.xy = shadowCoords.xy * 0.5 + 0.5;
	shadowCoords.y = 1.0 - shadowCoords.y;
	shadowCoords.xy = shadowCoords.xy * 0.5 + 0.5 * float2(index % 2.0, floor(float(index) / 2.0));

    float bias = 0.00005;
    float shadowFactor = (shadowMap.SampleLevel(shadowMapSampler, shadowCoords.xy, 0).r >= shadowCoords.z - bias);

    return shadowFactor;
}

float rand(float2 uv)
{
	return frac(sin(dot(uv.xy, float2(12.9898, 78.233)) * 43758.5453));
}

[shader("fragment")]
FORWARD_PS_OUTPUT fragmentMain(FORWARD_PS_INPUT input)
{
    const float3 view = passConstants.cameraPosition - input.worldPosition;
    const float3 eye = normalize(view);

	Intersection intersection;
	intersection.uv = input.uv;
    intersection.normal = normalize(input.normal);
    intersection.tangent = input.tangent;
	
	Material material;
	evaluateMaterial(material, intersection);

	float3 radiance = float3(0.0, 0.0, 0.0);

	radiance += material.Surface.integrateEmission(eye);

	//radiance += sceneConstants.ambientColor * surface.albedo;

	EnvironmentMap environment;
	environment.texture = environmentMap;
	environment.sampler = environmentSampler;
	environment.mipLevels = sceneConstants.environmentMipLevels;

    radiance += material.Surface.integrateEnvironment(eye, environment);

    // point lights
    for (int i = 0; i < sceneConstants.pointLightCount; i++)
    {
        float3 lightVector = sceneConstants.pointLights[i].position - input.worldPosition;
        float lightDistance = length(lightVector);

        LightSample lightSample;
        lightSample.direction = lightVector / lightDistance;
        lightSample.radiance = sceneConstants.pointLights[i].color * computeLightFalloff(lightDistance, sceneConstants.pointLights[i].radius);

        radiance += material.Surface.integrateSample(eye, lightSample);
    }

	float jitter = rand(input.uv);

	// spot lights
    float3 inScattering = float3(0.0, 0.0, 0.0);
    float stepLength = length(input.marchingStep);
    for (int j = 0; j < sceneConstants.spotLightCount; j++)
    {
        float3 lightVector = sceneConstants.spotLights[j].position - input.worldPosition;
        float lightDistance = length(lightVector);

        float shadowFactor = sampleShadowMap(j, input.worldPosition);

        LightSample lightSample;
        lightSample.direction = lightVector / lightDistance;
        lightSample.radiance = sceneConstants.spotLights[j].color * computeLightFalloff(lightDistance, sceneConstants.spotLights[j].radius) * shadowFactor;

        // angle falloff (scale and offset are precomputed on CPU according to the inner and outer angles)
        float angleFalloff = saturate(dot(-lightSample.direction, sceneConstants.spotLights[j].direction) * sceneConstants.spotLights[j].cosAngleScale + sceneConstants.spotLights[j].cosAngleOffset);
        angleFalloff *= angleFalloff; // more natural square attenuation
        lightSample.radiance *= angleFalloff;

        radiance += material.Surface.integrateSample(eye, lightSample);

        if (sceneConstants.spotLights[j].scattering == 0.0)
            continue;

		float3 samplePosition = passConstants.cameraPosition + input.marchingStep * jitter;
        float3 sampledScattering = float3(0.0, 0.0, 0.0);
        for (int k = 0; k < MARCHING_ITERATIONS; k++)
        {
            float3 lightVector2 = sceneConstants.spotLights[j].position - samplePosition;
            float lightDistance2 = length(lightVector2);
            float opticalDepth = distance(passConstants.cameraPosition, samplePosition) + lightDistance2;
            float angleFalloff2 = saturate(dot(-lightVector2 / lightDistance2, sceneConstants.spotLights[j].direction) * sceneConstants.spotLights[j].cosAngleScale + sceneConstants.spotLights[j].cosAngleOffset);
            angleFalloff2 *= angleFalloff2; // more natural square attenuation

			if (angleFalloff2 > 0.01)
			{
				float shadowFactor2 = sampleShadowMap(j, samplePosition);
				float3 radiance2 = sceneConstants.spotLights[j].color * computeLightFalloff(lightDistance, sceneConstants.spotLights[j].radius) * angleFalloff2 * shadowFactor2;

				//float density = exp(-samplePosition.z * 10.0);
				sampledScattering += stepLength * radiance2 * exp(-opticalDepth * 0.01);
			}

            samplePosition += input.marchingStep;
        }

        inScattering += sampledScattering * sceneConstants.spotLights[j].scattering;
    }

    float transmittance = exp(input.viewPosition.z * sceneConstants.mist);
    
	FORWARD_PS_OUTPUT output;

    output.radiance = float4(lerp(sceneConstants.ambientColor, radiance, transmittance) + inScattering, input.viewPosition.z);

    // estimate pixel movement from last frame
    float4 previousFrameClipSpacePosition = mul(input.worldToPreviousFrameClipSpaceMatrix, float4(input.worldPosition, 1.0));
    float2 frameMovement = (input.clipPosition.xy / input.clipPosition.w) - (previousFrameClipSpacePosition.xy / previousFrameClipSpacePosition.w);
    float2 clipSpaceMotion = frameMovement * sceneConstants.motionSpeedFactor;

	// clamp motion to tile size
    float2 screenSpaceMotion = clipSpaceMotion * passConstants.viewportSize.xy * 0.5;
	screenSpaceMotion /= max(1.0, length(screenSpaceMotion) / sceneConstants.motionBlurTileSize);
	
	// store half velocity
    output.motion = float4(0.5 * screenSpaceMotion, 0.0, 0.0);

	return output;
}
