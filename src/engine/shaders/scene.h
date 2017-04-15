#define MAX_LIGHT 8

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
    float cosAngleScale;
    float3 direction;
    float cosAngleOffset;
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
    float3 ambientColor;
    PointLightData pointLights[MAX_LIGHT];
    SpotLightData spotLights[MAX_LIGHT];
    float mist;
};
