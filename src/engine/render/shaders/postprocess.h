struct POSTPROCESS_PS_INPUT
{
	float4 pos: SV_POSITION;
	float2 uv: TEXCOORD0;
};

float computeLuminance(float3 color)
{
	return dot(color, float3(0.299, 0.587, 0.114));
}
