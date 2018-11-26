#pragma once

#include <string>
#include <vector>

#include <d3d11.h>

#include <engine/render/ShaderCache.h>
#include <engine/resource/Resource.h>

class AnimationData;
class Batch;
struct RenderSettings;
struct ShadowConstants;
class Image;

class Material: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        void getResources(std::vector<ID3D11ShaderResourceView *> &resources, std::vector<ID3D11SamplerState *> &samplers);

        ShaderCache::Hash getPrefixHash() const { return this->prefixHash; }

    private:
        AnimationData *animation = nullptr;
        ShaderCache::Hash prefixHash = { 0, 0 };
        std::vector<Image *> textures;
        ID3D11SamplerState *samplerState;
};
