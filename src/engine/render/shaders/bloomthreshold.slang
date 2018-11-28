import streams.geometry2d;

import parameters.scene;
import parameters.pass;
import parameters.bloom;

import color;

#define SAMPLE_COUNT 13

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

	float3 color = 0.0;

	float2 offsets[SAMPLE_COUNT] = {
		float2(-0.5, -0.5),
		float2(-0.5, 0.5),
		float2(0.5, 0.5),
		float2(0.5, -0.5),
		float2(-1.0, -1.0),
		float2(-1.0, 1.0),
		float2(1.0, 1.0),
		float2(1.0, -1.0),
		float2(-1.0, 0.0),
		float2(1.0, 0.0),
		float2(0.0, -1.0),
		float2(0.0, 1.0),
		float2(0.0, 0.0)
	};

	float weights[SAMPLE_COUNT] = {
		0.5 / 4.0,
		0.5 / 4.0,
		0.5 / 4.0,
		0.5 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 4.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125 / 2.0,
		0.125
	};

	[unroll]
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		float3 colorSample = bloomParameters.radianceTexture.Sample(bloomParameters.radianceSampler, input.uv + offsets[i] * passParameters.constants.viewportSize.zw).rgb;
		float luminance = computeLuminance(colorSample);
		float luminanceWeight = 1.0 / (1.0 + luminance);
		float thresholdWeight = smoothstep(bloomParameters.constants.threshold - 1.0, bloomParameters.constants.threshold, luminance);
		color += weights[i] * colorSample * luminanceWeight * thresholdWeight;
	}

	output.color = float4(color, 1.0);

	return output;
}
