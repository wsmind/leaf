#pragma once

#include <string>
#include <vector>

#include <engine/glm/glm.hpp>

#include <engine/AnimationPlayer.h>
#include <engine/Mesh.h>
#include <engine/Resource.h>

class AnimationData;
class RenderList;

class Scene: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        void updateAnimation(float time);

        void fillRenderList(RenderList *renderList) const;

    private:
        struct MeshInstance
        {
            Mesh *mesh;

            // transform
            glm::vec3 position;
            glm::vec3 orientation; /// XYZ Euler
            glm::vec3 scale;

            AnimationData *animation;

            MeshInstance()
                : mesh(nullptr)
                , animation(nullptr)
            {}
        };
        std::vector<MeshInstance *> instances;

        AnimationPlayer animationPlayer;
};
