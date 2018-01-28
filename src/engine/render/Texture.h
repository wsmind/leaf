#pragma once

#include <string>

#include <engine/render/Device.h>
#include <engine/resource/Resource.h>

class EnvironmentMap;
class Image;

class Texture: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Texture(): image(nullptr), environmentMap(nullptr) {}

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

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
        EnvironmentMap *environmentMap;
};
