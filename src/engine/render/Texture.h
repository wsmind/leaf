#pragma once

#include <string>

#include <engine/render/Device.h>
#include <engine/resource/Resource.h>
#include <engine/resource/ResourceWatcher.h>

class FrameGraph;
class Image;

class Texture: public Resource, public ResourceWatcher
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Texture(): image(nullptr), environmentMap(nullptr) {}

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        // used to get notified when an environment map changes and we need to rebake
        // precomputed BRDF integration
        virtual void onResourceUpdated(Resource *resource) override;

        // frame update for dynamic textures
        void update(FrameGraph *frameGraph);

        ID3D11SamplerState *getSamplerState() const { return this->samplerState; }
        ID3D11ShaderResourceView *getSRV() const;

    private:
        enum TextureType
        {
            TextureType_Image,
            TextureType_EnvironmentMap
        };
        TextureType type;

        ID3D11SamplerState *samplerState;

        // type-specific data
        Image *image;
        Image *environmentMap;

        // flag if the envmap prefiltering needs to be rebaked
        bool environmentMapDirty = false;
};
