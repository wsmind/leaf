#include "gamma.h"

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
