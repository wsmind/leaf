#include "equirectangular.h"
#include "pass.h"
#include "scene.h"
#include "unlit.h"

UNLIT_PS_OUTPUT main(UNLIT_PS_INPUT input)
{
    UNLIT_PS_OUTPUT output;

    float3 radiance = unlitConstants.emissive * emissiveMap.Sample(emissiveSampler, input.uv).rgb;
    output.radiance = float4(radiance, input.viewPosition.z);

    // estimate pixel movement from last frame
    float4 previousFrameClipSpacePosition = mul(input.worldToPreviousFrameClipSpaceMatrix, float4(input.worldPosition, 1.0));
    float2 frameMovement = (input.clipPosition.xy / input.clipPosition.w) - (previousFrameClipSpacePosition.xy / previousFrameClipSpacePosition.w);
    float2 clipSpaceMotion = frameMovement * sceneConstants.motionSpeedFactor;

	// clamp motion to tile size
	float2 screenSpaceMotion = clipSpaceMotion * passConstants.viewportSize.xy * 0.5;
	screenSpaceMotion /= max(1.0, length(screenSpaceMotion) / sceneConstants.motionBlurTileSize);
	
	// store half velocity
    output.motion = float4(0.5 * screenSpaceMotion, 0.0, 0.0);

	return output;
}
