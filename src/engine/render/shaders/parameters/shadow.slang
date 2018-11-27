#include "../constants/ShadowConstants.h"

cbuffer ShadowConstants
{
    ShadowConstants shadowConstants;
};

Texture2D shadowMap;
SamplerState shadowMapSampler;
