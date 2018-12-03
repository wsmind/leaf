import streams.depthonly;

import parameters.scene;
import parameters.pass;

struct VertexOutput
{
    float4 position : SV_POSITION;
};

[shader("vertex")]
VertexOutput vertexMain(InstancedMesh mesh)
{
    VertexOutput output;

    float4 worldPosition = mul(mesh.transformMatrix, float4(mesh.vertex.position, 1.0));
    float4 viewPosition = mul(passParameters.constants.viewMatrix, worldPosition);
    output.position = mul(passParameters.constants.projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}

[shader("fragment")]
void fragmentMain(VertexOutput input)
{
}
