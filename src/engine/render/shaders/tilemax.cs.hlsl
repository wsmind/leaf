#include "scene.h"

Texture2D<float4> motionTexture: register(t0);

RWTexture2D<float4> tileMax: register(u0);

float2 vmax(float2 v1, float2 v2)
{
    if (dot(v1, v1) > dot(v2, v2))
        return v1;
    else
        return v2;
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID)
{
	int tileSize = int(sceneConstants.motionBlurTileSize);

    float2 tileMotion = float2(0.0, 0.0);
    for (int y = 0; y < tileSize; y++)
    {
        for (int x = 0; x < tileSize; x++)
        {
            float2 motionSample = motionTexture.Load(uint3(dispatchThreadId.xy * tileSize + uint2(x, y), 0)).xy;
            tileMotion = vmax(tileMotion, motionSample);
        }
    }

    tileMax[dispatchThreadId.xy] = float4(tileMotion, 0.0, 0.0);
}
