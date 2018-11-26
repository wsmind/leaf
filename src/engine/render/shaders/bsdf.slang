import environment;

struct LightSample
{
    float3 direction;
    float3 radiance;
};

interface Bsdf
{
	float3 integrateEmission(float3 eye);

	float3 integrateSample(float3 eye, LightSample lightSample);

	float3 integrateEnvironment(float3 eye, EnvironmentMap environment);
};

struct NullBsdf: Bsdf
{
	// this member variable is useless, but removing it crashes the Slang compiler :)
	float3 __unused;

	float3 integrateEmission(float3 eye)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 integrateSample(float3 eye, LightSample lightSample)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 integrateEnvironment(float3 eye, EnvironmentMap environment)
	{
		return float3(0.0, 0.0, 0.0);
	}
};
NullBsdf nullBsdf;

struct LambertDiffuse: Bsdf
{
	float3 albedo;
	float3 normal;

	float3 integrateEmission(float3 eye)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 integrateSample(float3 eye, LightSample lightSample)
	{
		float3 diffuse = albedo / 3.14159265;

		float dotNL = saturate(dot(normal, lightSample.direction));
		return dotNL * lightSample.radiance * diffuse;
	}

	float3 integrateEnvironment(float3 eye, EnvironmentMap environment)
	{
		float3 diffuse = sampleEnvironmentDiffuse(environment, normal) * albedo;
		
		return diffuse;
	}
};

float g1v(float dotNV, float k)
{
    return 1.0 / (dotNV * (1.0 - k) + 1.0);
}

struct PrincipledBsdf: Bsdf
{
	float3 albedo;
	float3 normal;
	float3 specularColor;
	float roughness;

	float3 integrateEmission(float3 eye)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 integrateSample(float3 eye, LightSample lightSample)
	{
		const float3 h = normalize(eye + lightSample.direction);

		// precompute all cosines
		float dotLH = saturate(dot(lightSample.direction, h));
		float dotNH = saturate(dot(normal, h));
		float dotNL = saturate(dot(normal, lightSample.direction));
		float dotNV = saturate(dot(normal, eye));

		// simple lambert for diffuse
		float3 diffuse = albedo / 3.141592;

		// schlick fresnel approximation
		float3 fresnel = specularColor + (1.0 - specularColor) * pow(1.0 - dotLH, 5.0);

		float alpha = roughness * roughness;
		float alphaSquared = alpha * alpha;
		alphaSquared = max(0.0000001, alphaSquared); // avoid division by zero

		// GGX normal distribution
		float denominator = dotNH * dotNH * (alphaSquared - 1.0) + 1.0;
		float normalDistribution = alphaSquared / (3.141592 * denominator * denominator);

		// schlick approximation for visibility factor
		float k = alpha * 0.5;
		float visibility = g1v(dotNL, k) * g1v(dotNV, k);

		// cook-torrance microfacet model
		float3 specular = fresnel * normalDistribution * visibility;

		return dotNL * lightSample.radiance * (diffuse + specular);
	}

	float3 integrateEnvironment(float3 eye, EnvironmentMap environment)
	{
		float3 diffuse = sampleEnvironmentDiffuse(environment, normal) * albedo;
		
		float3 specularDirection = reflect(-eye, normal);
		float3 specular = sampleEnvironmentSpecular(environment, specularDirection, roughness) * specularColor;
		
		return diffuse + specular;
	}
};

struct EmissionBsdf: Bsdf
{
	float3 color;

	float3 integrateEmission(float3 eye)
	{
		return color;
	}

	float3 integrateSample(float3 eye, LightSample lightSample)
	{
		return float3(0.0, 0.0, 0.0);
	}

	float3 integrateEnvironment(float3 eye, EnvironmentMap environment)
	{
		return float3(0.0, 0.0, 0.0);
	}
};
