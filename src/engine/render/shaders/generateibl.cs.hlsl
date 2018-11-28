#include "equirectangular.slang"

cbuffer __unused : register(b0)
{
    // temp fix (the whole file will be replaced soon anyway)
    // just make sure that pass constants are in register (b1)
    float __plop;
};

#include "pass.h"

Texture2D<float4> environmentMap: register(t0);

RWTexture2D<float4> output: register(u0);

static const float PI = 3.1415926535;

// distribution from http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), radicalInverse_VdC(i));
}

// filtering from http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

float3 ImportanceSampleGGX(float2 Xi, float Roughness, float3 N)
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt((1 - Xi.y) / (1 + (a*a - 1) * Xi.y));
    float SinTheta = sqrt(1 - CosTheta * CosTheta);
    float3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 TangentX = normalize(cross(UpVector, N));
    float3 TangentY = cross(N, TangentX);

    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 PrefilterEnvMap(float Roughness, float3 R)
{
    float3 N = R;
    float3 V = R;

    float3 PrefilteredColor = 0;
    float TotalWeight = 0;

    const uint NumSamples = 48 + 2000 * Roughness;
    for (uint i = 0; i < NumSamples; i++)
    {
        float2 Xi = Hammersley(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = 2 * dot(V, H) * H - V;
        float NoL = saturate(dot(N, L));
        if (NoL > 0)
        {
            PrefilteredColor += environmentMap.Load(uint3(directionToEquirectangularUV(L) * passConstants.viewportSize.xy, 0)).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    float2 targetSize = float2(passConstants.viewMatrix[0][0], passConstants.viewMatrix[1][0]);
    float roughness = passConstants.viewMatrix[2][0];

    float3 direction = equirectangularUVToDirection(dispatchThreadId.xy / targetSize);
    //float3 radiance = environmentMap.Load(uint3(directionToEquirectangularUV(direction) * float2(3200, 1600), 0)).rgb;
    float3 radiance = PrefilterEnvMap(roughness, direction).rgb;

	output[dispatchThreadId.xy] = float4(radiance, 0.0);
}
