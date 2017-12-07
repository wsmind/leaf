#include "pass.h"
#include "scene.h"
#include "shared.h"

Texture2D<float4> albedoTexture: register(t0);
SamplerState albedoSampler: register(s0);

Texture2D<float4> normalTexture: register(t1);
SamplerState normalSampler: register(s1);

Texture2D<float4> metalnessTexture: register(t2);
SamplerState metalnessSampler: register(s2);

Texture2D<float4> roughnessTexture: register(t3);
SamplerState roughnessSampler: register(s3);

struct PS_OUTPUT
{
	float4 gbuffer0: SV_TARGET0;
	float4 gbuffer1: SV_TARGET1;
};

float g1v(float dotNV, float k)
{
    return 1.0 / (dotNV * (1.0 - k) + 1.0);
}

PS_OUTPUT main(GBUFFER_PS_INPUT input)
{
	PS_OUTPUT output;

    const float3 light = normalize(float3(1.0, 1.0, 1.0));
    const float3 view = passConstants.cameraPosition - input.worldPosition;
    const float3 eye = normalize(view);
    const float3 normal = normalize(input.normal);
    const float3 h = normalize(eye + light);

    // compute TBN for normal mapping (algorithm from http://www.thetenthplanet.de/archives/1180)

    // get edge vectors of the pixel triangle
    float3 dp1 = ddx(view);
    float3 dp2 = ddy(view);
    float2 duv1 = ddx(input.uv);
    float2 duv2 = ddy(input.uv);

    // solve the linear system
    float3 dp2perp = cross(dp2, normal);
    float3 dp1perp = cross(normal, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    // construct a scale-invariant frame
    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    float3x3 TBN = float3x3(T * invmax, B * invmax, normal);

    // compute normal after normal map perturbation, in world space
    float3 tangentNormal = normalize(normalTexture.Sample(normalSampler, input.uv).rgb * 2.0 - 1.0);
    float3 pertubatedNormal = mul(tangentNormal, TBN);

    float3 albedo = albedoTexture.Sample(albedoSampler, input.uv).rgb;
    float metalness = metalnessTexture.Sample(metalnessSampler, input.uv).r;
    float roughness = roughnessTexture.Sample(roughnessSampler, input.uv).r;

    // blend between dielectric and metal
    float3 specularColor = metalness * lerp(metalness.xxx, albedo, metalness);
    float3 finalAlbedo = albedo * (1.0 - metalness);

    // precompute all cosines
    float dotLH = saturate(dot(light, h));
    float dotNH = saturate(dot(pertubatedNormal, h));
    float dotNL = saturate(dot(pertubatedNormal, light));
    float dotNV = saturate(dot(pertubatedNormal, eye));

    // simple lambert for diffuse
    float3 diffuse = dotNL * finalAlbedo;

    // schlick fresnel approximation
    float3 fresnel = specularColor + (1.0 - specularColor) * pow(1.0 - dotNV, 5.0);

    float alpha = roughness * roughness;
    float alphaSquared = alpha * alpha;

    // GGX normal distribution
    float denominator = dotNH * dotNH * (alphaSquared - 1.0) + 1.0;
    float normalDistribution = alphaSquared / (3.141592 * denominator * denominator);

    // schlick approximation for visibility factor
    float k = alpha * 0.5;
    float visibility = g1v(dotNL, k) * g1v(dotNV, k);

    // cook-torrance microfacet model
    float3 specular = fresnel * normalDistribution * visibility;

    float3 color = diffuse + specular;

    // gamma correction
    //color = sqrt(color);

	output.gbuffer0 = float4(pertubatedNormal, metalness);
	output.gbuffer1 = float4(albedo, 0.0f);

	return output;
}
