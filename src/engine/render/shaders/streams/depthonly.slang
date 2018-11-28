struct Vertex
{
	float3 position: POSITION;
	float3 normal: NORMAL;
	float4 tangent: TANGENT;
	float2 uv: TEXCOORD;
};

struct InstancedMesh
{
	Vertex vertex;

	float4x4 transformMatrix: TRANSFORM;
};
