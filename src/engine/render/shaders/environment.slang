import equirectangular;

struct EnvironmentMap
{
	Texture2D texture;
	SamplerState sampler;
	uint mipLevels;
};

float3 sampleEnvironmentRadiance(EnvironmentMap environment, float3 direction)
{
	float2 uv = directionToEquirectangularUV(direction);
	return environment.texture.SampleLevel(environment.sampler, uv, 0.0).rgb;
}

float3 sampleEnvironmentDiffuse(EnvironmentMap environment, float3 direction)
{
	float2 uv = directionToEquirectangularUV(direction);
	return environment.texture.SampleLevel(environment.sampler, uv, environment.mipLevels - 5.0).rgb;
}

float3 sampleEnvironmentSpecular(EnvironmentMap environment, float3 direction, float roughness)
{
	float2 uv = directionToEquirectangularUV(direction);
	return environment.texture.SampleLevel(environment.sampler, uv, roughness * environment.mipLevels).rgb;
}
