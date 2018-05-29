#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct UnlitConstants
{
    float3 emissive;
    float _padding;

    float2 uvScale;
    float2 uvOffset;
};
