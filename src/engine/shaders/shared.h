struct PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 coord: TEXCOORD0;
};

struct BACKGROUND_PS_INPUT
{
    float4 pos: SV_POSITION;
    float3 worldPosition: POSITION0;
};

struct BASIC_PS_INPUT
{
    float4 position: SV_POSITION;
    float2 uv: TEXCOORD0;
};

struct GBUFFER_PS_INPUT
{
	float4 position: SV_POSITION;
    float3 worldPosition: POSITION0;
    nointerpolation float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};
