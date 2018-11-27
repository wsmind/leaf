#include "../constants/ShadowConstants.h"

struct ShadowParameters
{
    ShadowConstants constants;
	Texture2D texture;
	SamplerState sampler;
};

ParameterBlock<ShadowParameters> shadowParameters;
