struct SdfGbufferParameters
{
	Texture2D normalTexture;
	SamplerState normalSampler;
};

ParameterBlock<SdfGbufferParameters> sdfGbufferParameters;
