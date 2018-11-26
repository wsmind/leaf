import bsdf;
import geometry;

float cast_to_float(float3 value)
{
	return value.x;
}

float cast_to_float(float4 value)
{
	return value.x;
}

float3 cast_to_float3(float value)
{
	return value.xxx;
}

float3 cast_to_float3(float4 value)
{
	return value.xyz;
}

float4 cast_to_float4(float value)
{
	return value.xxxx;
}

float4 cast_to_float4(float3 value)
{
	return float4(value, 0.0);
}

typedef NullBsdf BSDF_DEFAULT_OUTPUT_TYPE;

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

void BSDF_DIFFUSE(in BSDF_DIFFUSE_input input, out BSDF_DIFFUSE_output output, Intersection intersection)
{
	LambertDiffuse brdf;
	brdf.albedo = input.Color.rgb;

	output.BSDF = brdf;
}

typedef LambertDiffuse BSDF_DIFFUSE_OUTPUT_TYPE;

struct BSDF_PRINCIPLED_input
{
	float4 BaseColor;

	float Subsurface;
	float3 SubsurfaceRadius;
	float4 SubsurfaceColor;

	float Metallic;
	float Specular;
	float SpecularTint;
	float Roughness;

	float Anisotropic;
	float AnisotropicRotation;

	float Sheen;
	float SheenTint;

	float Clearcoat;
	float ClearcoatRoughness;

	float IOR;

	float Transmission;
	float TransmissionRoughness;

	float3 Normal;
	float3 ClearcoatNormal;
	float3 Tangent;
};

struct BSDF_PRINCIPLED_output
{
	PrincipledBsdf BSDF;
};

void BSDF_PRINCIPLED(in BSDF_PRINCIPLED_input input, out BSDF_PRINCIPLED_output output, Intersection intersection)
{
	PrincipledBsdf bsdf;

    // blend between dielectric and metal
    float3 albedo = input.BaseColor.rgb * (1.0 - input.Metallic);
    float3 specularColor = lerp(float3(0.04, 0.04, 0.04), input.BaseColor.rgb, input.Metallic);

    bsdf.albedo = albedo;
    bsdf.normal = input.Normal;
    bsdf.specularColor = specularColor;
    bsdf.roughness = input.Roughness * 0.9;

	output.BSDF = bsdf;
}

typedef PrincipledBsdf BSDF_PRINCIPLED_OUTPUT_TYPE;

struct TEX_IMAGE_input
{
	float3 Vector;
};

struct TEX_IMAGE_output
{
	float4 Color;
	float Alpha;
};

void TEX_IMAGE(in TEX_IMAGE_input input, out TEX_IMAGE_output output, Texture2D texture, SamplerState sampler, Intersection intersection)
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

void TEX_COORD(in TEX_COORD_input input, out TEX_COORD_output output, Intersection intersection)
{
	output.Generated = float3(0.0, 0.0, 0.0);
	output.Normal = float3(0.0, 0.0, 0.0);
	output.UV = float3(intersection.uv, 0.0);
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

void INVERT(in INVERT_input input, out INVERT_output output, Intersection intersection)
{
	output.Color = lerp(input.Color, 1.0 - input.Color, input.Fac);
}

struct GAMMA_input
{
	float4 Color;
	float Gamma;
};

struct GAMMA_output
{
	float4 Color;
};

void GAMMA(in GAMMA_input input, out GAMMA_output output, Intersection intersection)
{
	output.Color = pow(input.Color, input.Gamma.xxxx);
}

struct NORMAL_MAP_input
{
	float Strength;
	float4 Color;
};

struct NORMAL_MAP_output
{
	float3 Normal;
};

void NORMAL_MAP(in NORMAL_MAP_input input, out NORMAL_MAP_output output, Intersection intersection)
{
    // compute TBN frame for normal mapping
    float3x3 TBN = float3x3(intersection.tangent, intersection.bitangent, intersection.normal);

    // compute normal after normal map perturbation, in world space
    float3 tangentSpaceNormal = input.Color.rgb * 2.0 - 1.0;
    float3 perturbedNormal = mul(tangentSpaceNormal, TBN);

	// blend geometric and shading normals
	output.Normal = normalize(lerp(intersection.normal, perturbedNormal, input.Strength));
}
