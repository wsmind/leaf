interface Bsdf
{
	float3 evaluate(float3 wo, float3 wi);
};

struct DefaultBsdf: Bsdf
{
	// this member variable is useless, but removing it crashes the Slang compiler :)
	float3 __unused;

	float3 evaluate(float3 wo, float3 wi)
	{
		return float3(0.0, 0.0, 0.0);
	}
};

struct LambertDiffuse: Bsdf
{
	float3 albedo;

	float3 evaluate(float3 wo, float3 wi)
	{
		return albedo / 3.14159265;
	}
};

DefaultBsdf defaultBsdf;
