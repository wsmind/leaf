import streams.instancedmesh;

import parameters.scene;
import parameters.pass;
import parameters.sdfgbuffer;
import parameters.shadow;
import parameters.environment;

import bsdf;
import environment;

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION0;
    float3 viewPosition : POSITION1;
    float3 marchingStep : POSITION2;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
	noperspective float2 uv : TEXCOORD0;
    float4 clipPosition : TEXCOORD1;
	float4x4 worldToPreviousFrameClipSpaceMatrix : TEXCOORD2;
};

struct PixelOutput
{
    float4 radiance : SV_TARGET0;
    float4 motion : SV_TARGET1;
};

#define MARCHING_ITERATIONS 64

[shader("vertex")]
VertexOutput vertexMain(InstancedMesh mesh)
{
    VertexOutput output;

    float2 uv = mesh.vertex.uv;

    float4 worldPosition = mul(mesh.modelMatrix, float4(mesh.vertex.position, 1.0));
    float4 viewPosition = mul(passParameters.constants.viewMatrix, worldPosition);
    output.position = mul(passParameters.constants.projectionMatrix, viewPosition);
   
    // hack; GL to DX clip space
    output.position.z = (output.position.z + output.position.w) * 0.5;

    output.worldPosition = worldPosition.xyz;
    output.viewPosition = viewPosition.xyz;
    output.marchingStep = (output.worldPosition - passParameters.constants.cameraPosition) / MARCHING_ITERATIONS;
    output.normal = mul(mesh.normalMatrix, mesh.vertex.normal);
    output.tangent = float4(mul(mesh.normalMatrix, mesh.vertex.tangent.xyz), mesh.vertex.tangent.w);
    output.uv = (output.position.xy / output.position.w) * 0.5 + 0.5;
	output.uv.y = 1.0 - output.uv.y;
    output.clipPosition = output.position;

	output.worldToPreviousFrameClipSpaceMatrix = mesh.worldToPreviousFrameClipSpaceMatrix;

    return output;
}

float computeLightFalloff(float distance, float radius)
{
    // see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
    float numerator = saturate(1.0 - pow(distance / radius, 4.0));
    numerator *= numerator;
    return numerator / (distance * distance + 1.0);
}

float sampleShadowMap(int index, float3 worldPosition)
{
    float4 shadowCoords = mul(shadowParameters.constants.lightMatrix[index], float4(worldPosition, 1.0));
    shadowCoords.z = (shadowCoords.z + shadowCoords.w) * 0.5; // hack; GL to DX clip space
    shadowCoords /= shadowCoords.w;
	shadowCoords.xy = shadowCoords.xy * 0.5 + 0.5;
	shadowCoords.y = 1.0 - shadowCoords.y;
	shadowCoords.xy = shadowCoords.xy * 0.5 + 0.5 * float2(index % 2.0, floor(float(index) / 2.0));

    float bias = 0.00005;
    float shadowFactor = (shadowParameters.texture.SampleLevel(shadowParameters.sampler, shadowCoords.xy, 0).r >= shadowCoords.z - bias);

    return shadowFactor;
}

float rand(float2 uv)
{
	return frac(sin(dot(uv.xy, float2(12.9898, 78.233)) * 43758.5453));
}

[shader("fragment")]
PixelOutput fragmentMain(VertexOutput input)
{
	float4 normalDepth = sdfGbufferParameters.normalTexture.Sample(sdfGbufferParameters.normalSampler, input.uv);
    float3 direction = normalize(input.worldPosition - passParameters.constants.cameraPosition);
	
	float3 normal = normalize(normalDepth.xyz);
	float viewSpaceDepth = normalDepth.w;
	float3 worldPosition = viewSpaceDepth * direction + passParameters.constants.cameraPosition;
	
	Intersection intersection;
	intersection.uv = worldPosition.xy;
    intersection.normal = normal;
    intersection.tangent = input.tangent;
	
	Material material;
	evaluateMaterial(material, intersection);

    const float3 view = passParameters.constants.cameraPosition - worldPosition;
    const float3 eye = normalize(view);
	
	float3 radiance = float3(0.0, 0.0, 0.0);

	radiance += material.Surface.integrateEmission(eye);

	//radiance += sceneParameters.constants.ambientColor * surface.albedo;

	EnvironmentMap environment;
	environment.texture = environmentMap;
	environment.sampler = environmentSampler;
	environment.mipLevels = sceneParameters.constants.environmentMipLevels;

    radiance += material.Surface.integrateEnvironment(eye, environment);

    // point lights
    for (int i = 0; i < sceneParameters.constants.pointLightCount; i++)
    {
        float3 lightVector = sceneParameters.constants.pointLights[i].position - worldPosition;
        float lightDistance = length(lightVector);

        LightSample lightSample;
        lightSample.direction = lightVector / lightDistance;
        lightSample.radiance = sceneParameters.constants.pointLights[i].color * computeLightFalloff(lightDistance, sceneParameters.constants.pointLights[i].radius);

        radiance += material.Surface.integrateSample(eye, lightSample);
    }

	float jitter = rand(input.uv);

	// spot lights
    float3 inScattering = float3(0.0, 0.0, 0.0);
    float stepLength = length(input.marchingStep);
    for (int j = 0; j < sceneParameters.constants.spotLightCount; j++)
    {
        float3 lightVector = sceneParameters.constants.spotLights[j].position - worldPosition;
        float lightDistance = length(lightVector);

        float shadowFactor = sampleShadowMap(j, worldPosition);

        LightSample lightSample;
        lightSample.direction = lightVector / lightDistance;
        lightSample.radiance = sceneParameters.constants.spotLights[j].color * computeLightFalloff(lightDistance, sceneParameters.constants.spotLights[j].radius) * shadowFactor;

        // angle falloff (scale and offset are precomputed on CPU according to the inner and outer angles)
        float angleFalloff = saturate(dot(-lightSample.direction, sceneParameters.constants.spotLights[j].direction) * sceneParameters.constants.spotLights[j].cosAngleScale + sceneParameters.constants.spotLights[j].cosAngleOffset);
        angleFalloff *= angleFalloff; // more natural square attenuation
        lightSample.radiance *= angleFalloff;

        radiance += material.Surface.integrateSample(eye, lightSample);

        if (sceneParameters.constants.spotLights[j].scattering == 0.0)
            continue;

		float3 samplePosition = passParameters.constants.cameraPosition + input.marchingStep * jitter;
        float3 sampledScattering = float3(0.0, 0.0, 0.0);
        for (int k = 0; k < MARCHING_ITERATIONS; k++)
        {
            float3 lightVector2 = sceneParameters.constants.spotLights[j].position - samplePosition;
            float lightDistance2 = length(lightVector2);
            float opticalDepth = distance(passParameters.constants.cameraPosition, samplePosition) + lightDistance2;
            float angleFalloff2 = saturate(dot(-lightVector2 / lightDistance2, sceneParameters.constants.spotLights[j].direction) * sceneParameters.constants.spotLights[j].cosAngleScale + sceneParameters.constants.spotLights[j].cosAngleOffset);
            angleFalloff2 *= angleFalloff2; // more natural square attenuation

			if (angleFalloff2 > 0.01)
			{
				float shadowFactor2 = sampleShadowMap(j, samplePosition);
				float3 radiance2 = sceneParameters.constants.spotLights[j].color * computeLightFalloff(lightDistance, sceneParameters.constants.spotLights[j].radius) * angleFalloff2 * shadowFactor2;

				//float density = exp(-samplePosition.z * 10.0);
				sampledScattering += stepLength * radiance2 * exp(-opticalDepth * 0.01);
			}

            samplePosition += input.marchingStep;
        }

        inScattering += sampledScattering * sceneParameters.constants.spotLights[j].scattering;
    }

    float transmittance = exp(-viewSpaceDepth * sceneParameters.constants.mist);
    
	PixelOutput output;

    output.radiance = float4(lerp(sceneParameters.constants.ambientColor, radiance, transmittance) + inScattering, viewSpaceDepth);
	//output.radiance = normalDepth;//float4(input.uv, 0.0, 0.0);

    // estimate pixel movement from last frame
    float4 previousFrameClipSpacePosition = mul(input.worldToPreviousFrameClipSpaceMatrix, float4(worldPosition, 1.0));
    float2 frameMovement = (input.clipPosition.xy / input.clipPosition.w) - (previousFrameClipSpacePosition.xy / previousFrameClipSpacePosition.w);
    float2 clipSpaceMotion = frameMovement * sceneParameters.constants.motionSpeedFactor;

	// clamp motion to tile size
    float2 screenSpaceMotion = clipSpaceMotion * passParameters.constants.viewportSize.xy * 0.5;
	screenSpaceMotion /= max(1.0, length(screenSpaceMotion) / sceneParameters.constants.motionBlurTileSize);
	
	// disable for now
	screenSpaceMotion = float2(0.0, 0.0);
	
	// store half velocity
    output.motion = float4(0.5 * screenSpaceMotion, 0.0, 0.0);

	return output;
}
