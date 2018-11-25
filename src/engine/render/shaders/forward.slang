#include "equirectangular.h"
#include "pass.h"
#include "scene.h"
#include "shadows.h"

#include "constants/StandardConstants.h"

cbuffer StandardConstants : register(b2)
{
    StandardConstants standardConstants;
};

/*Texture2D<float4> baseColorMap: register(t0);
SamplerState baseColorSampler: register(s0);

Texture2D<float4> normalMap: register(t1);
SamplerState normalSampler: register(s1);

Texture2D<float4> metallicMap: register(t2);
SamplerState metallicSampler: register(s2);

Texture2D<float4> roughnessMap: register(t3);
SamplerState roughnessSampler : register(s3);

Texture2D shadowMap: register(t4);
SamplerState shadowMapSampler: register(s4);

Texture2D<float4> environmentMap: register(t5);
SamplerState environmentSampler : register(s5);*/

struct STANDARD_PS_INPUT
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

struct STANDARD_PS_OUTPUT
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
STANDARD_PS_INPUT vertexMain(VS_INPUT input)
{
    STANDARD_PS_INPUT output;

    float2 uv = input.uv * standardConstants.uvScale + standardConstants.uvOffset;

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

/*float g1v(float dotNV, float k)
{
    return 1.0 / (dotNV * (1.0 - k) + 1.0);
}

struct SurfaceProperties
{
    float3 albedo;
    float3 normal;
    float3 specularColor;
    float roughness;
};

struct LightProperties
{
    float3 direction;
    float3 incomingRadiance;
};

float3 computeShading(SurfaceProperties surface, LightProperties light, float3 eye)
{
    const float3 h = normalize(eye + light.direction);

    // precompute all cosines
    float dotLH = saturate(dot(light.direction, h));
    float dotNH = saturate(dot(surface.normal, h));
    float dotNL = saturate(dot(surface.normal, light.direction));
    float dotNV = saturate(dot(surface.normal, eye));

    // simple lambert for diffuse
    float3 diffuse = surface.albedo / 3.141592;

    // schlick fresnel approximation
    float3 fresnel = surface.specularColor + (1.0 - surface.specularColor) * pow(1.0 - dotLH, 5.0);

    float alpha = surface.roughness * surface.roughness;
    float alphaSquared = alpha * alpha;
	alphaSquared = max(0.0000001, alphaSquared); // avoid division by zero

    // GGX normal distribution
    float denominator = dotNH * dotNH * (alphaSquared - 1.0) + 1.0;
    float normalDistribution = alphaSquared / (3.141592 * denominator * denominator);

    // schlick approximation for visibility factor
    float k = alpha * 0.5;
    float visibility = g1v(dotNL, k) * g1v(dotNV, k);

    // cook-torrance microfacet model
    float3 specular = fresnel * normalDistribution * visibility;

    return dotNL * light.incomingRadiance * (diffuse + specular);
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
    float4 shadowCoords = mul(standardConstants.shadows.lightMatrix[index], float4(worldPosition, 1.0));
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

float3 computeEnvironmentIrradiance(SurfaceProperties surface)
{
    float2 uv = directionToEquirectangularUV(surface.normal);
    return environmentMap.SampleLevel(environmentSampler, uv, sceneConstants.environmentMipLevels - 5.0).rgb;
}

float3 computeEnvironmentRadiance(SurfaceProperties surface, float3 eye)
{
    float3 direction = reflect(-eye, surface.normal);
    float2 uv = directionToEquirectangularUV(direction);
    return environmentMap.SampleLevel(environmentSampler, uv, surface.roughness * sceneConstants.environmentMipLevels).rgb;
}*/

[shader("fragment")]
STANDARD_PS_OUTPUT fragmentMain(STANDARD_PS_INPUT input)
{
    STANDARD_PS_OUTPUT output;

	float2 intersection = input.uv;
	
	Material material;
	evaluateMaterial(material, intersection);
    float3 radiance = material.Surface.evaluate(float3(0.0, 0.0, 0.0), float3(0.0, 0.0, 0.0));

	output.radiance = float4(radiance, input.viewPosition.z);
	output.motion = float4(0.0, 0.0, 0.0, 0.0);

	return output;

    /*const float3 view = passConstants.cameraPosition - input.worldPosition;
    const float3 eye = normalize(view);
    const float3 normal = normalize(input.normal);
    const float3 tangent = normalize(input.tangent.xyz);
    const float3 bitangent = normalize(-input.tangent.w * cross(normal, tangent));

    // compute TBN frame for normal mapping
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    // compute normal after normal map perturbation, in world space
    float3 tangentNormal = normalize(normalMap.Sample(normalSampler, input.uv).rgb * 2.0 - 1.0);
    float3 perturbedNormal = mul(tangentNormal, TBN);

    float3 baseColor = standardConstants.baseColorMultiplier * baseColorMap.Sample(baseColorSampler, input.uv).rgb;
    float metallic = saturate(standardConstants.metallicOffset + metallicMap.Sample(metallicSampler, input.uv).r);
    float roughness = saturate(standardConstants.roughnessOffset + roughnessMap.Sample(roughnessSampler, input.uv).r);

    // blend between dielectric and metal
    float3 albedo = baseColor * (1.0 - metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), baseColor, metallic);

    SurfaceProperties surface;
    surface.albedo = albedo;
    surface.normal = perturbedNormal;
    surface.specularColor = specularColor;
    surface.roughness = roughness * 0.9;

	radiance += sceneConstants.ambientColor * surface.albedo;

    radiance += computeEnvironmentIrradiance(surface) * surface.albedo;
    radiance += computeEnvironmentRadiance(surface, eye) * surface.specularColor;

    // point lights
    for (int i = 0; i < sceneConstants.pointLightCount; i++)
    {
        float3 lightVector = sceneConstants.pointLights[i].position - input.worldPosition;
        float lightDistance = length(lightVector);

        LightProperties light;
        light.direction = lightVector / lightDistance;
        light.incomingRadiance = sceneConstants.pointLights[i].color * computeLightFalloff(lightDistance, sceneConstants.pointLights[i].radius);

        radiance += computeShading(surface, light, eye);
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

        LightProperties light;
        light.direction = lightVector / lightDistance;
        light.incomingRadiance = sceneConstants.spotLights[j].color * computeLightFalloff(lightDistance, sceneConstants.spotLights[j].radius) * shadowFactor;

        // angle falloff (scale and offset are precomputed on CPU according to the inner and outer angles)
        float angleFalloff = saturate(dot(-light.direction, sceneConstants.spotLights[j].direction) * sceneConstants.spotLights[j].cosAngleScale + sceneConstants.spotLights[j].cosAngleOffset);
        angleFalloff *= angleFalloff; // more natural square attenuation
        light.incomingRadiance *= angleFalloff;

        radiance += computeShading(surface, light, eye);

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

	return output;*/
}
