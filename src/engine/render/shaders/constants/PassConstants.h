#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

struct PassConstants
{
	float4x4 viewMatrix;
	float4x4 viewMatrixInverse;
	float4x4 projectionMatrix;
	float4x4 projectionMatrixInverse;
	float4x4 viewProjectionInverseMatrix;
	float3 cameraPosition;
    float _padding;
};
