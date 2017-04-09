struct PointLightData
{
    float3 position;
    float radius;
    float3 color;
};

struct SpotLightData
{
    float3 position;
    float radius;
    float3 color;
    float angle;
    float3 direction;
    float blend;
};

cbuffer SceneData : register(b0)
{
    float4x4 viewMatrix;
    float4x4 viewMatrixInverse;
    float4x4 projectionMatrix;
    float4x4 projectionMatrixInverse;
    float4x4 viewProjectionInverseMatrix;
    float3 cameraPosition;
    int pointLightCount;
    int spotLightCount;
    PointLightData pointLights[16];
    SpotLightData spotLights[16];
};
