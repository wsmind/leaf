import parameters.scene;
import parameters.pass;
import parameters.environment;

import environment;

struct VS_INPUT
{
	float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
};

struct BACKGROUND_PS_INPUT
{
    float4 pos: SV_POSITION;
    float3 worldPosition: POSITION0;
};

struct PS_OUTPUT
{
    float4 radiance : SV_TARGET0;
    float4 motion : SV_TARGET1;
};

[shader("vertex")]
BACKGROUND_PS_INPUT vertexMain(VS_INPUT input)
{
    BACKGROUND_PS_INPUT output;

    // force z to 1 to make sure the background is always behind everything
	output.pos = float4(input.pos.xy, 1.0, 1.0);
    output.worldPosition = mul(passParameters.constants.viewProjectionInverseMatrix, output.pos).xyz;

	return output;
}

[shader("fragment")]
PS_OUTPUT fragmentMain(BACKGROUND_PS_INPUT input)
{
	PS_OUTPUT output;

	EnvironmentMap environment;
	environment.texture = environmentMap;
	environment.sampler = environmentSampler;
	environment.mipLevels = sceneParameters.constants.environmentMipLevels;

    float3 direction = normalize(input.worldPosition);
	float3 radiance = 0.0; //sampleEnvironmentRadiance(environment, direction);
	
	output.radiance = float4(radiance, 0.0);
	output.motion = float4(0.0, 0.0, 0.0, 0.0);

	return output;
}
