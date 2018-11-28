#include "../constants/BloomConstants.h"

struct BloomParameters
{
	BloomConstants constants;

	Texture2D radianceTexture;
	SamplerState radianceSampler;
};

ParameterBlock<BloomParameters> bloomParameters;
