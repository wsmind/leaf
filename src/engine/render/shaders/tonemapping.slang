import color;

float3 linearToneMapping(float3 color)
{
    // apply only gamma correction
    return linearToGamma(color);
}

float3 reinhardToneMapping(float3 color)
{
    color = color / (1.0 + color);
    return linearToGamma(color);
}

float3 filmicToneMapping(float3 color)
{
    float3 x = max(0, color - 0.004);
    return (x * (6.2 * x + 0.5)) / (x * (6.2 * x + 1.7) + 0.06);
}
