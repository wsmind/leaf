#ifdef __cplusplus
#pragma once
#endif

#include "ShaderTypes.h"

#define MAX_LIGHT 8

struct PointLightData
{
	float3 position;
	float radius;
	float3 color;
	float _padding;
};

struct SpotLightData
{
	float3 position;
	float radius;
	float3 color;
	float cosAngleScale;
	float3 direction;
	float cosAngleOffset;
	float scattering;
	float _padding0;
	float _padding1;
	float _padding2;
};

struct SceneConstants
{
    float3 ambientColor;
	float motionSpeedFactor; // shutter speed / delta time
	float motionBlurTileSize; // in pixels
    float mist;
    int pointLightCount;
	int spotLightCount;
	PointLightData pointLights[MAX_LIGHT];
	SpotLightData spotLights[MAX_LIGHT];
	float focusDistance;
	float environmentMipLevels;
	float _padding1;
	float _padding2;
};
