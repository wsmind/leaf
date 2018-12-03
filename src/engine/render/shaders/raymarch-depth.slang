import streams.depthonly;

import parameters.scene;
import parameters.pass;

// these can be overriden in the custom SDF code
// (it will be appended as a prefix to this file)

#ifndef LEAF_RAYMARCH_ITERATIONS
#define LEAF_RAYMARCH_ITERATIONS 64
#endif

#ifndef LEAF_RAYMARCH_THRESHOLD
#define LEAF_RAYMARCH_THRESHOLD 0.01
#endif

struct VertexOutput
{
    float4 position : SV_Position;
    float3 worldPosition;
};

struct PixelOutput
{
	float depth : SV_Depth;
};

[shader("vertex")]
VertexOutput vertexMain(InstancedMesh mesh)
{
    VertexOutput output;

    output.worldPosition = mul(mesh.transformMatrix, float4(mesh.vertex.position, 1.0));
    float4 viewPosition = mul(passParameters.constants.viewMatrix, output.worldPosition);
    output.position = mul(passParameters.constants.projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}

bool traceRay(vec3 position, vec3 direction)
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

[shader("fragment")]
void fragmentMain(VertexOutput input)
{
	vec3 position = input.worldPosition;
	vec3 direction = normalize(position - passParameters.constants.cameraPosition);
	
	// raymarch the custom SDF
	bool hit = traceRay(position, direction);
	
	if (hit)
	{
		// reproject to clip space and write SDF depth
		float4 viewPosition = mul(passParameters.constants.viewMatrix, position);
		float4 clipSpacePosition = mul(passParameters.constants.projectionMatrix, viewPosition);
		
		// hack; GL to DX clip space
		clipSpacePosition.z = (clipSpacePosition.z + clipSpacePosition.w) * 0.5;
		
		// manually apply the conversion to non-homogeneous coordinates
		output.depth = clipSpacePosition.z / clipSpacePosition.w;
	}
	else
	{
		discard;
	}
}
