cbuffer ShadowConstants : register(b4)
{
    float4x4 lightMatrix[16];
};

Texture2D shadowMap: register(t4);
SamplerState shadowMapSampler : register(s4);
