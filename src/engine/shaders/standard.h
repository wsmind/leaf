struct STANDARD_PS_INPUT
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    nointerpolation float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};
