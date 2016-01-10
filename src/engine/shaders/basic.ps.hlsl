#include "shared.h"

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

float g1v(float dotNV, float k)
{
    return 1.0 / (dotNV * (1.0 - k) + 1.0);
}

PS_OUTPUT main(BASIC_PS_INPUT input)
{
	PS_OUTPUT output;

    const float3 light = normalize(float3(1.0, 1.0, 1.0));
    const float3 eye = normalize(cameraPosition - input.worldPosition);
    const float3 normal = normalize(input.normal);
    const float3 h = normalize(eye + light);

    /*float d = saturate(dot(normal, light));
    float s = saturate(pow(dot(normal, h), 80.0));
    float3 blinn_phong = diffuse * d + s;*/

    // blend between dielectric and metal
    float3 specularColor = metalness * lerp(metalness.xxx, albedo, metalness);
    float3 finalAlbedo = albedo * (1.0 - metalness);

    // precompute all cosines
    float dotLH = saturate(dot(light, h));
    float dotNH = saturate(dot(normal, h));
    float dotNL = saturate(dot(normal, light));
    float dotNV = saturate(dot(normal, eye));

    // simple lambert for diffuse
    float3 diffuse = dotNL * finalAlbedo;

    // schlick fresnel approximation
    float3 fresnel = specularColor + (1.0 - specularColor) * pow(1.0 - dotNV, 5.0);

    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;

    // GGX normal distribution
    float denominator = dotNH * dotNH * (alphaSquared - 1.0) + 1.0;
    float normalDistribution = alphaSquared / (3.141592 * denominator * denominator);

    // schlick approximation for geometry factor
    float k = alpha * 0.5;
    float visibility = g1v(dotNL, k) * g1v(dotNV, k);

    // cook-torrance microfacet model
    float3 specular = fresnel * normalDistribution * visibility;

    float3 color = diffuse + specular;

    // gamma correction
    color = sqrt(color);

	output.color = float4(color, 1.0);

	return output;
}
