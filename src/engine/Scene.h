#pragma once

#include <string>
#include <vector>

#include <engine/glm/glm.hpp>

#include <engine/Mesh.h>
#include <engine/Resource.h>

class Scene: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

    private:
        struct MeshInstance
        {
            Mesh *mesh;
            glm::mat4 transform;
        };
        std::vector<MeshInstance> instances;
};
