#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct StandardConstants
{
    float3 baseColorMultiplier;
    float metallicOffset;
    float roughnessOffset;
    float3 emissive;
};
