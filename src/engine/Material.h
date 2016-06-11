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
            glm::vec3 albedo;
            float emit;
            float metalness;
            float roughness;
            float _padding[2];
        };
        #pragma pack(pop)

        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        const MaterialData &getMaterialData() const { return this->data; }

        void bindTextures() const;

    private:
        MaterialData data;

        Texture *albedoTexture;
        Texture *normalTexture;
        Texture *metalnessTexture;
        Texture *roughnessTexture;

        AnimationData *animation = nullptr;
};
