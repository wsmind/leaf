struct PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 coord: TEXCOORD0;
};

struct BASIC_PS_INPUT
{
	float4 position: SV_POSITION;
	float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

cbuffer SceneState: register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float time;
};
