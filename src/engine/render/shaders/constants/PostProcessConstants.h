#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct PostProcessConstants
{
	float pixellateDivider;
	float vignetteSize;
	float vignettePower;
    float abberationStrength;
};
