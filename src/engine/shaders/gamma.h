float3 gammaToLinear(float3 color)
{
    return pow(color, 2.2);
}

float3 linearToGamma(float3 color)
{
    return pow(color, 1.0 / 2.2);
}
