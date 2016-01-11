struct PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 coord: TEXCOORD0;
};

struct BASIC_PS_INPUT
{
	float4 position: SV_POSITION;
    float3 worldPosition: POSITION0;
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

cbuffer SceneData: register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float3 cameraPosition;
    float time;
};

cbuffer MaterialData: register(b1)
{
    float3 albedo2;
    float metalness;
    float roughness;
};

cbuffer InstanceData: register(b2)
{
    float4x4 modelMatrix;
};
