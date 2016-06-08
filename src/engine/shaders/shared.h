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
    float3 normal: NORMAL;
    float2 uv: TEXCOORD;
};

cbuffer SceneData: register(b0)
{
    float4x4 viewMatrix;
    float4x4 viewMatrixInverse;
    float4x4 projectionMatrix;
    float4x4 projectionMatrixInverse;
    float4x4 viewProjectionInverseMatrix;
    float3 cameraPosition;
};

cbuffer MaterialData: register(b1)
{
    // UNUSED (textures are used instead)
    float3 albedo2;
    float metalness2;
    float roughness2;
};

cbuffer InstanceData: register(b2)
{
    float4x4 modelMatrix;
};
