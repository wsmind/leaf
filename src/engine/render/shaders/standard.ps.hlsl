#include "equirectangular.h"
#include "pass.h"
#include "scene.h"
#include "shadows.h"
#include "standard.h"

float g1v(float dotNV, float k)
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
    return environmentMap.SampleLevel(environmentSampler, uv, sceneConstants.environmentMipLevels - 3.0).rgb * 100.0;
}

float3 computeEnvironmentRadiance(SurfaceProperties surface, float3 eye)
{
    float3 direction = reflect(-eye, surface.normal);
    float2 uv = directionToEquirectangularUV(direction);
    return environmentMap.SampleLevel(environmentSampler, uv, surface.roughness * sceneConstants.environmentMipLevels).rgb;
}

STANDARD_PS_OUTPUT main(STANDARD_PS_INPUT input)
{
    STANDARD_PS_OUTPUT output;

    float3 radiance = standardConstants.emissive;

    const float3 view = passConstants.cameraPosition - input.worldPosition;
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

    radiance += computeEnvironmentIrradiance(surface) * surface.albedo / 3.14159265;
    radiance += computeEnvironmentRadiance(surface, eye) / 3.14159265;

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
    for (int i = 0; i < sceneConstants.spotLightCount; i++)
    {
        float3 lightVector = sceneConstants.spotLights[i].position - input.worldPosition;
        float lightDistance = length(lightVector);

        float shadowFactor = sampleShadowMap(i, input.worldPosition);

        LightProperties light;
        light.direction = lightVector / lightDistance;
        light.incomingRadiance = sceneConstants.spotLights[i].color * computeLightFalloff(lightDistance, sceneConstants.spotLights[i].radius) * shadowFactor;

        // angle falloff (scale and offset are precomputed on CPU according to the inner and outer angles)
        float angleFalloff = saturate(dot(-light.direction, sceneConstants.spotLights[i].direction) * sceneConstants.spotLights[i].cosAngleScale + sceneConstants.spotLights[i].cosAngleOffset);
        angleFalloff *= angleFalloff; // more natural square attenuation
        light.incomingRadiance *= angleFalloff;

        radiance += computeShading(surface, light, eye);

        if (sceneConstants.spotLights[i].scattering == 0.0)
            continue;

		float3 samplePosition = passConstants.cameraPosition + input.marchingStep * jitter;
        float3 sampledScattering = float3(0.0, 0.0, 0.0);
        for (int k = 0; k < MARCHING_ITERATIONS; k++)
        {
            float3 lightVector2 = sceneConstants.spotLights[i].position - samplePosition;
            float lightDistance2 = length(lightVector2);
            float opticalDepth = distance(passConstants.cameraPosition, samplePosition) + lightDistance2;
            float angleFalloff2 = saturate(dot(-lightVector2 / lightDistance2, sceneConstants.spotLights[i].direction) * sceneConstants.spotLights[i].cosAngleScale + sceneConstants.spotLights[i].cosAngleOffset);
            angleFalloff2 *= angleFalloff2; // more natural square attenuation

			if (angleFalloff2 > 0.01)
			{
				float shadowFactor2 = sampleShadowMap(i, samplePosition);
				float3 radiance2 = sceneConstants.spotLights[i].color * computeLightFalloff(lightDistance, sceneConstants.spotLights[i].radius) * angleFalloff2 * shadowFactor2;

				//float density = exp(-samplePosition.z * 10.0);
				sampledScattering += stepLength * radiance2 * exp(-opticalDepth * 0.01);
			}

            samplePosition += input.marchingStep;
        }

        inScattering += sampledScattering * sceneConstants.spotLights[i].scattering;
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

	return output;
}
