import parameters.scene;
import parameters.pass;

struct VS_INPUT
{
    float3 pos: POSITION;
    float3 normal: NORMAL;
    float4 tangent: TANGENT;
    float2 uv: TEXCOORD;
	float4x4 transformMatrix: TRANSFORM;
};

struct DEPTHONLY_PS_INPUT
{
    float4 position : SV_POSITION;
};

[shader("vertex")]
DEPTHONLY_PS_INPUT vertexMain(VS_INPUT input)
{
    DEPTHONLY_PS_INPUT output;

    float4 worldPosition = mul(input.transformMatrix, float4(input.pos, 1.0));
    float4 viewPosition = mul(passParameters.constants.viewMatrix, worldPosition);
    output.position = mul(passParameters.constants.projectionMatrix, viewPosition);

    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    return output;
}

[shader("fragment")]
void fragmentMain(DEPTHONLY_PS_INPUT input)
{
}
