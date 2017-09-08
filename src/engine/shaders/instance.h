cbuffer InstanceData : register(b2)
{
    float4x4 modelMatrix;
    float4x4 previousFrameModelMatrix;
    float3x3 normalMatrix;
};
