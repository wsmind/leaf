#include "scene.h"
#include "standard.h"

struct STANDARD_PS_OUTPUT
{
    float4 radiance: SV_TARGET;
};

Texture2D<float4> albedoTexture: register(t0);
SamplerState albedoSampler: register(s0);

Texture2D<float4> normalTexture: register(t1);
SamplerState normalSampler: register(s1);

Texture2D<float4> metalnessTexture: register(t2);
SamplerState metalnessSampler: register(s2);

Texture2D<float4> roughnessTexture: register(t3);
SamplerState roughnessSampler: register(s3);

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
    float3 diffuse = dotNL * surface.albedo / 3.141592;

    // schlick fresnel approximation
    float3 fresnel = surface.specularColor + (1.0 - surface.specularColor) * pow(1.0 - dotNV, 5.0);

    float alpha = surface.roughness * surface.roughness;
    float alphaSquared = alpha * alpha;

    // GGX normal distribution
    float denominator = dotNH * dotNH * (alphaSquared - 1.0) + 1.0;
    float normalDistribution = alphaSquared / (3.141592 * denominator * denominator);

    // schlick approximation for visibility factor
    float k = alpha * 0.5;
    float visibility = g1v(dotNL, k) * g1v(dotNV, k);

    // cook-torrance microfacet model
    float3 specular = fresnel * normalDistribution * visibility;

    return light.incomingRadiance * (diffuse + specular);
}

float computeLightFalloff(float distance, float radius)
{
    // see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
    float numerator = saturate(1.0 - pow(distance / radius, 4.0));
    numerator *= numerator;
    return numerator / (distance * distance + 1.0);
}

STANDARD_PS_OUTPUT main(STANDARD_PS_INPUT input)
{
    STANDARD_PS_OUTPUT output;

    float3 radiance = float3(0.0, 0.0, 0.0);

    const float3 view = cameraPosition - input.worldPosition;
    const float3 eye = normalize(view);
    const float3 normal = normalize(input.normal);
    const float3 tangent = normalize(input.tangent.xyz);
    const float3 bitangent = normalize(input.tangent.w * cross(normal, tangent));

    // compute TBN frame for normal mapping
    float3x3 TBN = float3x3(tangent, bitangent, normal);

    // compute normal after normal map perturbation, in world space
    float3 tangentNormal = normalize(normalTexture.Sample(normalSampler, input.uv).rgb * 2.0 - 1.0);
    float3 perturbedNormal = mul(tangentNormal, TBN);

    float3 albedo = albedoTexture.Sample(albedoSampler, input.uv).rgb;
    float metalness = metalnessTexture.Sample(metalnessSampler, input.uv).r;
    float roughness = roughnessTexture.Sample(roughnessSampler, input.uv).r;

    // blend between dielectric and metal
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), albedo, metalness);
    albedo *= (1.0 - metalness);

    SurfaceProperties surface;
    surface.albedo = albedo;
    surface.normal = perturbedNormal;
    surface.specularColor = specularColor;
    surface.roughness = roughness;

    for (int i = 0; i < lightCount; i++)
    {
        float3 lightVector = lights[i].position - input.worldPosition;
        float lightDistance = length(lightVector);

        LightProperties light;
        light.direction = lightVector / lightDistance;
        light.incomingRadiance = lights[i].color * computeLightFalloff(lightDistance, lights[i].radius);

        radiance += computeShading(surface, light, eye);
    }

    output.radiance = float4(radiance, 1.0);

	return output;
}
