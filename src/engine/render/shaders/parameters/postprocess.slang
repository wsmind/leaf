#include "../constants/PostProcessConstants.h"

struct PostprocessParameters
{
	PostProcessConstants constants;

	Texture2D radianceTexture;
	SamplerState radianceSampler;
};

ParameterBlock<PostprocessParameters> postprocessParameters;
