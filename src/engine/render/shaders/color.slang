float3 gammaToLinear(float3 color)
{
    return pow(color, 2.2);
}

float3 linearToGamma(float3 color)
{
    return pow(color, 1.0 / 2.2);
}

float computeLuminance(float3 color)
{
	return dot(color, float3(0.299, 0.587, 0.114));
}
