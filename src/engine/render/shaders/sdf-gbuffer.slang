import streams.distancefield;

import parameters.scene;
import parameters.pass;

// these can be overriden in the custom SDF code
// (it will be appended as a prefix to this file)

#ifndef LEAF_RAYMARCH_ITERATIONS
#define LEAF_RAYMARCH_ITERATIONS 100
#endif

#ifndef LEAF_RAYMARCH_THRESHOLD
#define LEAF_RAYMARCH_THRESHOLD 0.01
#endif

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPosition : POSITION0;

	float4x4 modelMatrixInverse: MODELMATRIXINVERSE;
	float3x3 normalMatrix: NORMALMATRIX;
	float3x3 normalMatrixInverse: NORMALMATRIXINVERSE;
};

struct PixelOutput
{
	float4 normal : SV_Target;
	float depth : SV_Depth;
};

[shader("vertex")]
VertexOutput vertexMain(InstancedMesh mesh)
{
    VertexOutput output;

    float4 worldPosition = mul(mesh.modelMatrix, float4(mesh.vertex.position, 1.0));
    float4 viewPosition = mul(passParameters.constants.viewMatrix, worldPosition);
    output.position = mul(passParameters.constants.projectionMatrix, viewPosition);

	output.worldPosition = worldPosition.xyz;

	output.modelMatrixInverse = mesh.modelMatrixInverse;
	output.normalMatrix = mesh.normalMatrix;
	output.normalMatrixInverse = mesh.normalMatrixInverse;

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}

bool traceRay(inout float3 position, float3 direction)
{
	for (int i = 0; i < LEAF_RAYMARCH_ITERATIONS; i++)
	{
		float d = map(position);
		if (d < LEAF_RAYMARCH_THRESHOLD)
			return true;
		position += d * direction;
	}
	return false;
}

float3 computeNormal(float3 position)
{
	float2 eps = float2(LEAF_RAYMARCH_THRESHOLD, 0.0);
	return normalize(float3(
		map(position + eps.xyy) - map(position - eps.xyy),
		map(position + eps.yxy) - map(position - eps.yxy),
		map(position + eps.yyx) - map(position - eps.yyx)
	));
}

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
	PixelOutput output;
	output.depth = 1.0;
	output.normal = float4(0.0, 0.0, 0.0, 0.0);

	float3 position = input.worldPosition;
	float3 direction = position - passParameters.constants.cameraPosition;
	
	// start marching from the camera position to allow the distance field to spill
	// on closer geometry
	position = passParameters.constants.cameraPosition;

	// apply model transform before raymarching
	position = mul(input.modelMatrixInverse, float4(position, 1.0)).xyz;
	direction = normalize(mul(input.normalMatrixInverse, direction));

	// raymarch the custom SDF
	bool hit = traceRay(position, direction);
	
	if (hit)
	{
		// reproject to clip space and write SDF depth
		float4 viewPosition = mul(passParameters.constants.viewMatrix, float4(position, 1.0));
		float4 clipSpacePosition = mul(passParameters.constants.projectionMatrix, viewPosition);
		
		// hack; GL to DX clip space
		clipSpacePosition.z = (clipSpacePosition.z + clipSpacePosition.w) * 0.5;
		
		// manually apply the conversion to non-homogeneous coordinates
		output.depth = clipSpacePosition.z / clipSpacePosition.w;

		float3 normal = computeNormal(position);
		float viewSpaceDepth = distance(passParameters.constants.cameraPosition, position);
		output.normal = float4(mul(input.normalMatrix, normal), viewSpaceDepth);
	}
	else
	{
		discard;
	}

	return output;
}
