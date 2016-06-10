#pragma once

#include <engine/glm/glm.hpp>
#include <engine/AnimationPlayer.h>

struct cJSON;

template <typename DataType>
class SceneNode
{
    public:
        SceneNode(const cJSON *json);
        ~SceneNode();

        void registerAnimation(AnimationPlayer *player) const;
        void unregisterAnimation(AnimationPlayer *player) const;

        glm::mat4 computeTransformMatrix() const;

        DataType *getData() const;

    private:
        // transform
        glm::vec3 position;
        glm::vec3 orientation; /// XYZ Euler
        glm::vec3 scale;

        // transform animation
        AnimationData *animation;

        // custom data attached to this node
        DataType *data;
};

template <typename DataType>
SceneNode<DataType>::SceneNode(const cJSON *json)
{
    this->animation = nullptr;

    std::string dataName = cJSON_GetObjectItem(json, "data")->valuestring;
    this->data = ResourceManager::getInstance()->requestResource<DataType>(dataName);

    cJSON *position = cJSON_GetObjectItem(json, "position");
    this->position = glm::vec3(cJSON_GetArrayItem(position, 0)->valuedouble, cJSON_GetArrayItem(position, 1)->valuedouble, cJSON_GetArrayItem(position, 2)->valuedouble);

    cJSON *orientation = cJSON_GetObjectItem(json, "orientation");
    this->orientation = glm::vec3(cJSON_GetArrayItem(orientation, 0)->valuedouble, cJSON_GetArrayItem(orientation, 1)->valuedouble, cJSON_GetArrayItem(orientation, 2)->valuedouble);

    cJSON *scale = cJSON_GetObjectItem(json, "scale");
    this->scale = glm::vec3(cJSON_GetArrayItem(scale, 0)->valuedouble, cJSON_GetArrayItem(scale, 1)->valuedouble, cJSON_GetArrayItem(scale, 2)->valuedouble);

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("location", (float *)&this->position);
        properties.add("rotation_euler", (float *)&this->orientation);
        properties.add("scale", (float *)&this->scale);

        this->animation = new AnimationData(animation, properties);
    }
}

template <typename DataType>
SceneNode<DataType>::~SceneNode()
{
    delete this->animation;
    ResourceManager::getInstance()->releaseResource(this->data);
}

template <typename DataType>
void SceneNode<DataType>::registerAnimation(AnimationPlayer *player) const
{
    if (this->animation)
        player->registerAnimation(this->animation);
}

template <typename DataType>
void SceneNode<DataType>::unregisterAnimation(AnimationPlayer *player) const
{
    if (this->animation)
        player->unregisterAnimation(this->animation);
}

template <typename DataType>
glm::mat4 SceneNode<DataType>::computeTransformMatrix() const
{
    glm::mat4 rotation = glm::eulerAngleZ(this->orientation.z) * glm::eulerAngleY(this->orientation.y) * glm::eulerAngleX(this->orientation.x);
    return glm::translate(glm::mat4(), this->position) * rotation * glm::scale(glm::mat4(), this->scale);
}

template <typename DataType>
DataType *SceneNode<DataType>::getData() const
{
    return this->data;
}
