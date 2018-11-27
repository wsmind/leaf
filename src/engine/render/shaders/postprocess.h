#include "constants/PostProcessConstants.h"

// legacy cbuffer declaration
cbuffer PostProcessConstants: register(b2)
{
	PostProcessConstants postProcessConstants;
};

struct POSTPROCESS_VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : TEXCOORD;
};

struct POSTPROCESS_PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct POSTPROCESS_PS_OUTPUT
{
    float4 color : SV_TARGET;
};

float computeLuminance(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

Texture2D<float4> radianceTexture: register(t0);
SamplerState radianceSampler : register(s0);
