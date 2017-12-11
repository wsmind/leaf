#include "scene.h"

Texture2D<float4> tileMaxTexture: register(t0);

RWTexture2D<float4> neighborMaxTexture: register(u0);

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
	uint3 offsets = uint3(-1, 0, 1);

    float2 sample00 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.xx, 0)).xy;
	float2 sample10 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.yx, 0)).xy;
	float2 sample20 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.zx, 0)).xy;

	float2 sample01 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.xy, 0)).xy;
	float2 sample11 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.yy, 0)).xy;
	float2 sample21 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.zy, 0)).xy;

	float2 sample02 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.xz, 0)).xy;
	float2 sample12 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.yz, 0)).xy;
	float2 sample22 = tileMaxTexture.Load(uint3(dispatchThreadId.xy + offsets.zz, 0)).xy;

	float2 max0 = vmax(vmax(sample00, sample10), sample20);
	float2 max1 = vmax(vmax(sample01, sample11), sample21);
	float2 max2 = vmax(vmax(sample02, sample12), sample22);

	float2 tileMotion = vmax(vmax(max0, max1), max2);
	neighborMaxTexture[dispatchThreadId.xy] = float4(tileMotion, 0.0, 0.0);
}
