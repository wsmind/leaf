float2 directionToEquirectangularUV(float3 direction)
{
    // convert ray to equirectangular coordinates
    float u = (atan2(direction.y, -direction.x) / 3.141592) * 0.5 + 0.5;
    float v = (asin(-direction.z) / 1.570796) * 0.5 + 0.5;

    return float2(u, v);
}

float3 equirectangularUVToDirection(float2 uv)
{
    float longitude = (0.5 - uv.x) * 2.0 * 3.141592;
    float latitude = uv.y * 3.141592;

    float3 direction = float3(
        sin(latitude) * cos(longitude),
        sin(latitude) * sin(longitude),
        cos(latitude)
    );

    return direction;
}
