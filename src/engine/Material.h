#pragma once

#include <string>
#include <engine/glm/vec3.hpp>

#include <engine/Resource.h>

class AnimationData;
class Texture;

class Material: public Resource
{
    public:
        // packed in memory like on GPU, to do a direct copy to cbuffer
        #pragma pack(push)
        #pragma pack(16)
        struct MaterialData
        {
            glm::vec3 baseColorMultiplier;
            float metallicOffset;
            float roughnessOffset;
            float _padding[3];
        };
        #pragma pack(pop)

        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        const MaterialData &getMaterialData() const { return this->data; }

        void bindTextures() const;

    private:
        MaterialData data;

        Texture *baseColorMap;
        Texture *normalMap;
        Texture *metallicMap;
        Texture *roughnessMap;

        AnimationData *animation = nullptr;
};
