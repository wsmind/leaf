#pragma once

#include <d3d11.h>

class AnimationData;
class Batch;
class PropertyMapping;
struct RenderSettings;
struct ShadowConstants;
class Texture;

class Bsdf
{
    public:
        virtual ~Bsdf() {}

        virtual void registerAnimatedProperties(PropertyMapping &properties) {}
        virtual void setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants) {}
};
