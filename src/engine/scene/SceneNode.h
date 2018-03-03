#pragma once

#include <vector>

#include <engine/glm/glm.hpp>

#include <engine/scene/ParticleSystem.h>

struct cJSON;
class AnimationData;
class AnimationPlayer;
class Resource;
class RenderList;

class SceneNode
{
    public:
        SceneNode(const cJSON *json, const SceneNode *parent);
        ~SceneNode();

        void registerAnimation(AnimationPlayer *player) const;
        void unregisterAnimation(AnimationPlayer *player) const;

        bool isHidden() const { return this->hide == 1.0f; }

        // update current and last frame transforms, applying the parent transform
        // (parent must have been updated before to ensure correctness)
        void updateTransforms();

        const glm::mat4 &getCurrentTransform() const { return this->currentTransform; }
        const glm::mat4 &getPreviousFrameTransform() const { return this->previousFrameTransform; }

        // view transform is a special case because cameras need to ignore scaling
        glm::mat4 computeViewTransform() const;

        template <typename DataType>
        DataType *getData() const;

        bool hasParticleSystems() const { return this->particleSystems.size() > 0; }

        void updateParticles(float time);
        void fillParticleRenderList(RenderList *renderList) const;

    private:
        // transform
        glm::vec3 position;
        glm::vec3 orientation; // XYZ Euler
        glm::vec3 scale;

        // baked transform of the above fields, with parent transform applied
        glm::mat4 currentTransform;
        glm::mat4 previousFrameTransform;

        const SceneNode *parent;
        glm::mat4 parentMatrix;

        // transform animation
        AnimationData *animation;

        float hide;

        // custom data attached to this node
        Resource *data;

        std::vector<ParticleSystem *> particleSystems;
};

template <typename DataType>
DataType *SceneNode::getData() const
{
    return static_cast<DataType *>(this->data);
}
