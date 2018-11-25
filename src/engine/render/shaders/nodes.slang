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

void BSDF_DIFFUSE(in BSDF_DIFFUSE_input input, out BSDF_DIFFUSE_output output, float2 intersection)
{
	LambertDiffuse brdf;
	brdf.albedo = input.Color.rgb;

	output.BSDF = brdf;
}

typedef LambertDiffuse BSDF_DIFFUSE_OUTPUT_TYPE;

typedef DefaultBsdf BSDF_DEFAULT_OUTPUT_TYPE;

struct TEX_IMAGE_input
{
	float3 Vector;
};

struct TEX_IMAGE_output
{
	float4 Color;
	float Alpha;
};

void TEX_IMAGE(in TEX_IMAGE_input input, out TEX_IMAGE_output output, Texture2D texture, SamplerState sampler, float2 intersection)
{
	float4 pixel = texture.Sample(sampler, input.Vector.xy);
	output.Color = pixel;
	output.Alpha = pixel.a;
}

struct TEX_COORD_input
{
};

struct TEX_COORD_output
{
	float3 Generated;
	float3 Normal;
	float3 UV;
	float3 Object;
	float3 Camera;
	float3 Window;
	float3 Reflection;
};

void TEX_COORD(in TEX_COORD_input input, out TEX_COORD_output output, float2 intersection)
{
	output.Generated = float3(0.0, 0.0, 0.0);
	output.Normal = float3(0.0, 0.0, 0.0);
	output.UV = float3(intersection, 0.0);
	output.Object = float3(0.0, 0.0, 0.0);
	output.Camera = float3(0.0, 0.0, 0.0);
	output.Window = float3(0.0, 0.0, 0.0);
	output.Reflection = float3(0.0, 0.0, 0.0);
}

struct INVERT_input
{
	float Fac;
	float4 Color;
};

struct INVERT_output
{
	float4 Color;
};

void INVERT(in INVERT_input input, out INVERT_output output, float2 intersection)
{
	output.Color = lerp(input.Color, 1.0 - input.Color, input.Fac);
}
