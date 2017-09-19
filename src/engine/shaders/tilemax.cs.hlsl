Texture2D<float4> motionTexture: register(t0);

RWTexture2D<float4> tileMax;

SamplerState PointSampler
{
    Filter = MIN_MAG_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
    tileMax[dispatchThreadId.xy] = float4(dispatchThreadId.xy / 10.0, 0.0, 0.0);
}
