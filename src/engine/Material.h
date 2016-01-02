#pragma once

#include <string>
#include <engine/glm/vec3.hpp>

#include <engine/Resource.h>

class Material: public Resource
{
    public:
        // packed in memory like on GPU, to do a direct copy to cbuffer
        #pragma pack(push)
        #pragma pack(16)
        struct MaterialData
        {
            glm::vec3 diffuse;
        };
        #pragma pack(pop)

        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        const MaterialData &getMaterialData() const { return this->data; }

    private:
        MaterialData data;
};
