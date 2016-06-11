#pragma once

#include <string>
#include <vector>

#include <engine/glm/glm.hpp>

#include <engine/AnimationPlayer.h>
#include <engine/Camera.h>
#include <engine/Light.h>
#include <engine/Mesh.h>
#include <engine/Resource.h>
#include <engine/SceneNode.h>

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
        std::vector<SceneNode<Mesh> *> meshNodes;
        std::vector<SceneNode<Light> *> lightNodes;
        std::vector<SceneNode<Camera> *> cameraNodes;

        AnimationPlayer animationPlayer;
};
