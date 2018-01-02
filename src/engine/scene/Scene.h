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

	const RenderSettings &updateRenderSettings(int width, int height, bool overrideCamera = false, const glm::mat4 &viewMatrixOverride = glm::mat4(), const glm::mat4 &projectionMatrixOverride = glm::mat4());

    private:
		void updateCameraSettings(bool overrideCamera, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, float aspect);
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
