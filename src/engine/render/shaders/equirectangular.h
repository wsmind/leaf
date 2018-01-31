float2 directionToEquirectangularUV(float3 direction)
{
    // convert ray to equirectangular coordinates
    float u = (atan2(direction.y, -direction.x) / 3.141592) * 0.5 + 0.5;
    float v = (asin(-direction.z) / 1.570796) * 0.5 + 0.5;

    return float2(u, v);
}
