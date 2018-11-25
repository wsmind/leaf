import bsdf;

struct BSDF_DIFFUSE_input
{
	float4 Color;
	float Roughness;
	float3 Normal;
};

struct BSDF_DIFFUSE_output
{
	LambertDiffuse BSDF;
};

void BSDF_DIFFUSE(in BSDF_DIFFUSE_input input, out BSDF_DIFFUSE_output output)
{
	LambertDiffuse brdf;
	brdf.albedo = input.Color.rgb;

	output.BSDF = brdf;
}

typedef LambertDiffuse BSDF_DIFFUSE_OUTPUT_TYPE;

typedef DefaultBsdf BSDF_DEFAULT_OUTPUT_TYPE;
