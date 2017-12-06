#include "shared.h"

Texture2D<float4> gBuffer0Texture: register(t0);
SamplerState gBuffer0Sampler: register(s0);

Texture2D<float4> gBuffer1Texture: register(t1);
SamplerState gBuffer1Sampler: register(s1);

struct PS_OUTPUT
{
	float4 color: SV_TARGET;
};

/*float g1v(float dotNV, float k)
{
    return 1.0 / (dotNV * (1.0 - k) + 1.0);
}*/

PS_OUTPUT main(BASIC_PS_INPUT input)
{
	PS_OUTPUT output;

    /*const float3 light = normalize(float3(1.0, 1.0, 1.0));
    const float3 view = cameraPosition - input.worldPosition;
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
    color = sqrt(color);

	output.color = float4(color, 1.0);*/

    float4 g0 = gBuffer0Texture.Sample(gBuffer0Sampler, input.uv);
    float4 g1 = gBuffer1Texture.Sample(gBuffer1Sampler, input.uv);

    float3 normal = g0.xyz;
    float metalness = g0.w;
    float3 albedo = g1.xyz;
    //float roughness = g1.w;
    float emit = g1.w;

    const float3 light = normalize(float3(1.0, 1.0, 1.0));

    float dotNL = saturate(dot(normal, light));

    // simple lambert for diffuse
    float3 diffuse = dotNL * albedo;

    output.color = float4((diffuse + emit) * albedo, 1.0);

    return output;
}
