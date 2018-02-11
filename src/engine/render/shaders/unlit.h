#include "constants/UnlitConstants.h"

cbuffer UnlitConstants : register(b2)
{
    UnlitConstants unlitConstants;
};

Texture2D<float4> emissiveMap: register(t0);
SamplerState emissiveSampler: register(s0);

struct UNLIT_PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 viewPosition : POSITION1;
    float2 uv : TEXCOORD0;
    float4 clipPosition : TEXCOORD1;
	float4x4 worldToPreviousFrameClipSpaceMatrix : TEXCOORD2;
};

struct UNLIT_PS_OUTPUT
{
    float4 radiance : SV_TARGET0;
    float4 motion : SV_TARGET1;
};
