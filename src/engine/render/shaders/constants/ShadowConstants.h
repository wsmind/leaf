#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct ShadowConstants
{
	float4x4 lightMatrix[16];
};
