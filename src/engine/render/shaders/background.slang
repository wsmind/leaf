import streams.geometry2d;

import parameters.scene;
import parameters.pass;
import parameters.environment;

import environment;

struct VertexOutput
{
    float4 position: SV_POSITION;
    float3 worldPosition: POSITION0;
};

struct PixelOutput
{
    float4 radiance : SV_TARGET0;
    float4 motion : SV_TARGET1;
};

[shader("vertex")]
VertexOutput vertexMain(Geometry2D geometry)
{
    VertexOutput output;

    // force z to 1 to make sure the background is always behind everything
	output.position = float4(geometry.vertex.position.xy, 1.0, 1.0);
    output.worldPosition = mul(passParameters.constants.viewProjectionInverseMatrix, output.position).xyz;

	return output;
}

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
	PixelOutput output;

	EnvironmentMap environment;
	environment.texture = environmentMap;
	environment.sampler = environmentSampler;
	environment.mipLevels = sceneParameters.constants.environmentMipLevels;

    float3 direction = normalize(input.worldPosition);
	float3 radiance = sampleEnvironmentRadiance(environment, direction);
	
	output.radiance = float4(radiance, 0.0);
	output.motion = float4(0.0, 0.0, 0.0, 0.0);

	return output;
}
