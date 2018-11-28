import streams.geometry2d;

import parameters.scene;
import parameters.pass;
import parameters.bloom;

#define SAMPLE_COUNT 9

Texture2D accumulatorTexture;
SamplerState accumulatorSampler;

struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

struct PixelOutput
{
	float4 color: SV_TARGET;
};

[shader("vertex")]
VertexOutput vertexMain(Geometry2D geometry)
{
    VertexOutput output;
    output.position = float4(geometry.vertex.position.xy, 0.0, 1.0);
    output.uv = geometry.vertex.uv;

    return output;
}

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
	PixelOutput output;

	float3 color = bloomParameters.radianceTexture.Sample(bloomParameters.radianceSampler, input.uv).rgb;

	float radius = 1.0;

	float2 offsets[SAMPLE_COUNT] = {
		float2(-1.0, -1.0),
		float2(0.0, -1.0),
		float2(1.0, -1.0),
		float2(-1.0, 0.0),
		float2(0.0, 0.0),
		float2(1.0, 0.0),
		float2(-1.0, 1.0),
		float2(0.0, 1.0),
		float2(1.0, 1.0)
	};

	float weights[SAMPLE_COUNT] = {
		1.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0,
		2.0 / 16.0,
		4.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0,
		2.0 / 16.0,
		1.0 / 16.0
	};

	[unroll]
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		color += bloomParameters.constants.intensity * weights[i] * accumulatorTexture.Sample(accumulatorSampler, input.uv + offsets[i] * radius * passParameters.constants.viewportSize.zw).rgb;
	}

	output.color = float4(color, 1.0);

	return output;
}
