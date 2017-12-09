#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct ShadowConstants
{
	float4x4 lightMatrix[16];
};

struct StandardConstants
{
    float3 baseColorMultiplier;
    float metallicOffset;
    float roughnessOffset;
    float3 emissive;

	ShadowConstants shadows;
};
