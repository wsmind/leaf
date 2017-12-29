#pragma once

#include <string>
#include <vector>

#include <engine/glm/glm.hpp>

#include <engine/animation/AnimationPlayer.h>
#include <engine/render/Camera.h>
#include <engine/render/Light.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderSettings.h>
#include <engine/resource/Resource.h>
#include <engine/scene/SceneNode.h>

class AnimationData;
class RenderList;

class Scene : public Resource
{
    public:
    static const std::string resourceClassName;
    static const std::string defaultResourceData;

    Scene() : currentCamera(0) {}

    virtual void load(const unsigned char *buffer, size_t size) override;
    virtual void unload() override;

    void updateAnimation(float time);

    void updateTransforms();

    void fillRenderList(RenderList *renderList) const;
    void setupCamera(glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, float &shutterSpeed, float aspect) const;

	const RenderSettings &getRenderSettings() const { return this->renderSettings; }

    private:
        int findCurrentCamera(float time);

        std::vector<SceneNode *> nodes;

        // subsets of the above vector (same objects)
        std::vector<SceneNode *> meshNodes;
        std::vector<SceneNode *> lightNodes;
        std::vector<SceneNode *> cameraNodes;

        AnimationPlayer animationPlayer;

        struct Marker
        {
            int cameraIndex;
            float time;
        };
        std::vector<Marker> markers;

        int currentCamera; // computed through markers, defaults to activeCamera if no markers
        int activeCamera; // active camera at the time of export

		RenderSettings renderSettings;
};
