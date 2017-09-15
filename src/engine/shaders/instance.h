cbuffer InstanceData : register(b2)
{
    float4x4 modelMatrix;
    float4x4 worldToPreviousFrameClipSpaceMatrix;
    float3x3 normalMatrix;
};
