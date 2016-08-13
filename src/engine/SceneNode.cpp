#include <engine/SceneNode.h>

#include <engine/glm/glm.hpp>
#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtx/euler_angles.hpp>

#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/Camera.h>
#include <engine/Light.h>
#include <engine/Mesh.h>
#include <engine/PropertyMapping.h>
#include <engine/ResourceManager.h>

SceneNode::SceneNode(const cJSON *json, const SceneNode *parent)
    : parent(parent)
{
    this->animation = nullptr;

    std::string dataName = cJSON_GetObjectItem(json, "data")->valuestring;
    int dataType = cJSON_GetObjectItem(json, "type")->valueint;

    this->data = nullptr;
    switch (dataType)
    {
        case 0: this->data = ResourceManager::getInstance()->requestResource<Camera>(dataName); break;
        case 1: this->data = ResourceManager::getInstance()->requestResource<Mesh>(dataName); break;
        case 2: this->data = ResourceManager::getInstance()->requestResource<Light>(dataName); break;
    }

    cJSON *position = cJSON_GetObjectItem(json, "position");
    this->position = glm::vec3(cJSON_GetArrayItem(position, 0)->valuedouble, cJSON_GetArrayItem(position, 1)->valuedouble, cJSON_GetArrayItem(position, 2)->valuedouble);

    cJSON *orientation = cJSON_GetObjectItem(json, "orientation");
    this->orientation = glm::vec3(cJSON_GetArrayItem(orientation, 0)->valuedouble, cJSON_GetArrayItem(orientation, 1)->valuedouble, cJSON_GetArrayItem(orientation, 2)->valuedouble);

    cJSON *scale = cJSON_GetObjectItem(json, "scale");
    this->scale = glm::vec3(cJSON_GetArrayItem(scale, 0)->valuedouble, cJSON_GetArrayItem(scale, 1)->valuedouble, cJSON_GetArrayItem(scale, 2)->valuedouble);

    this->hide = (float)cJSON_GetObjectItem(json, "hide")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("location", (float *)&this->position);
        properties.add("rotation_euler", (float *)&this->orientation);
        properties.add("scale", (float *)&this->scale);
        properties.add("hide", &this->hide);

        this->animation = new AnimationData(animation, properties);
    }

    cJSON *parentMatrixJson = cJSON_GetObjectItem(json, "parentMatrix");
    if (parentMatrixJson)
    {
        this->parentMatrix = glm::mat4(
            cJSON_GetArrayItem(parentMatrixJson, 0)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 1)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 2)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 3)->valuedouble,
            cJSON_GetArrayItem(parentMatrixJson, 4)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 5)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 6)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 7)->valuedouble,
            cJSON_GetArrayItem(parentMatrixJson, 8)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 9)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 10)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 11)->valuedouble,
            cJSON_GetArrayItem(parentMatrixJson, 12)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 13)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 14)->valuedouble, cJSON_GetArrayItem(parentMatrixJson, 15)->valuedouble
        );
    }
}

SceneNode::~SceneNode()
{
    delete this->animation;
    ResourceManager::getInstance()->releaseResource(this->data);
}

void SceneNode::registerAnimation(AnimationPlayer *player) const
{
    if (this->animation)
        player->registerAnimation(this->animation);
}

void SceneNode::unregisterAnimation(AnimationPlayer *player) const
{
    if (this->animation)
        player->unregisterAnimation(this->animation);
}

glm::mat4 SceneNode::computeTransformMatrix() const
{
    glm::mat4 rotation = glm::eulerAngleZ(this->orientation.z) * glm::eulerAngleY(this->orientation.y) * glm::eulerAngleX(this->orientation.x);
    glm::mat4 transform = glm::translate(glm::mat4(), this->position) * rotation * glm::scale(glm::mat4(), this->scale);

    if (this->parent != nullptr)
        return this->parent->computeTransformMatrix() * this->parentMatrix * transform;

    return transform;
}
