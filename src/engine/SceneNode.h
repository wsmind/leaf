#pragma once

#include <engine/glm/glm.hpp>

struct cJSON;
class AnimationData;
class AnimationPlayer;
class Resource;

class SceneNode
{
    public:
        SceneNode(const cJSON *json, const SceneNode *parent);
        ~SceneNode();

        void registerAnimation(AnimationPlayer *player) const;
        void unregisterAnimation(AnimationPlayer *player) const;

        bool isHidden() const { return this->hide != 0.0f; }
        glm::mat4 computeTransformMatrix() const;

        template <typename DataType>
        DataType *getData() const;

    private:
        // transform
        glm::vec3 position;
        glm::vec3 orientation; /// XYZ Euler
        glm::vec3 scale;

        const SceneNode *parent;
        glm::mat4 parentMatrix;

        // transform animation
        AnimationData *animation;

        float hide;

        // custom data attached to this node
        Resource *data;
};

template <typename DataType>
DataType *SceneNode::getData() const
{
    return static_cast<DataType *>(this->data);
}
