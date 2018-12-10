float vmax(float3 v)
{
	return max(max(v.x, v.y), v.z);
}

float sdBox(float3 position, float3 size)
{
	float3 offset = abs(position) - size;
	return length(max(offset, 0.0)) + vmax(min(offset, 0.0));
}

float sdSphere(float3 position, float radius)
{
	return length(position) - radius;
}

float opUnion(float d1, float d2)
{
	return min(d1, d2);
}

float opIntersection(float d1, float d2)
{
	return max(d1, d2);
}

float opSubtract(float d1, float d2)
{
	return max(d1, -d2);
}
