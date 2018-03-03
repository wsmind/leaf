#include <engine/scene/SceneNode.h>

#include <engine/glm/glm.hpp>
#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtx/euler_angles.hpp>

#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>
#include <engine/render/Camera.h>
#include <engine/render/Light.h>
#include <engine/render/Mesh.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

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

    cJSON *particleSystems = cJSON_GetObjectItem(json, "particleSystems");
    if (particleSystems)
    {
        for (int i = 0; i < cJSON_GetArraySize(particleSystems); i++)
        {
            cJSON *ps = cJSON_GetArrayItem(particleSystems, i);
            this->particleSystems.push_back(new ParticleSystem(ps));
        }
    }

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

    if (this->data != nullptr)
        ResourceManager::getInstance()->releaseResource(this->data);

    for (auto *particleSystem : this->particleSystems)
        delete particleSystem;
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

void SceneNode::updateTransforms()
{
    // backup current transform
    this->previousFrameTransform = this->currentTransform;

    // node's own transform
    glm::mat4 rotation = glm::eulerAngleZ(this->orientation.z) * glm::eulerAngleY(this->orientation.y) * glm::eulerAngleX(this->orientation.x);
    glm::mat4 transform = glm::translate(glm::mat4(), this->position) * rotation * glm::scale(glm::mat4(), this->scale);

    // apply parent transform, if any
    if (this->parent != nullptr)
    {
        glm::mat4 parentTransform = this->parent->currentTransform * this->parentMatrix;
        transform = parentTransform * transform;
    }

    this->currentTransform = transform;
}

glm::mat4 SceneNode::computeViewTransform() const
{
    glm::mat4 rotation = glm::eulerAngleZ(this->orientation.z) * glm::eulerAngleY(this->orientation.y) * glm::eulerAngleX(this->orientation.x);
    glm::mat4 transform = glm::translate(glm::mat4(), this->position) * rotation;

    if (this->parent != nullptr)
    {
        glm::mat4 parentTransform = this->parent->computeViewTransform() * this->parentMatrix;

        // compensate for parent scale
        glm::vec3 scaledUnit = glm::mat3(parentTransform) * glm::vec3(1.0f, 0.0f, 0.0f);
        float parentScale = glm::length(scaledUnit);
        transform = transform * glm::scale(glm::mat4(), glm::vec3(1.0f / parentScale));

        transform = parentTransform * transform;
    }

    return transform;
}

void SceneNode::updateParticles(float time)
{
    for (auto *particleSystem : this->particleSystems)
        particleSystem->update(time);
}

void SceneNode::fillParticleRenderList(RenderList *renderList) const
{
    for (auto *particleSystem : this->particleSystems)
        particleSystem->fillRenderList(renderList);
}
