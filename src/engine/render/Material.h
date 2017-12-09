#pragma once

#include <string>

#include <d3d11.h>

#include <engine/glm/vec3.hpp>

#include <engine/render/shaders/constants/StandardConstants.h>
#include <engine/resource/Resource.h>

class AnimationData;
class Batch;
class Texture;

class Material: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        void setupBatch(Batch *batch, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants);

    private:
        StandardConstants constants;
        ID3D11Buffer *constantBuffer;

        Texture *baseColorMap;
        Texture *normalMap;
        Texture *metallicMap;
        Texture *roughnessMap;

        AnimationData *animation = nullptr;
};
