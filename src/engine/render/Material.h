#pragma once

#include <string>

#include <d3d11.h>

#include <engine/resource/Resource.h>

class AnimationData;
class Batch;
class Bsdf;
struct RenderSettings;
struct ShadowConstants;

class Material: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        void setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants);

    private:
        AnimationData *animation = nullptr;
        Bsdf *bsdf = nullptr;
};
