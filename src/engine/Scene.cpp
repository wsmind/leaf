#include <engine/Scene.h>

#include <cassert>
#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtx/euler_angles.hpp>

#include <engine/AnimationData.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"meshes\": [], \"lights\": []}";

void Scene::load(const cJSON *json)
{
    cJSON *meshesJson = cJSON_GetObjectItem(json, "meshes");
    cJSON *instanceJson = meshesJson->child;
    while (instanceJson)
    {
        std::string meshName = cJSON_GetObjectItem(instanceJson, "mesh")->valuestring;

        MeshInstance *instance = new MeshInstance;
        instance->mesh = ResourceManager::getInstance()->requestResource<Mesh>(meshName);

        cJSON *position = cJSON_GetObjectItem(instanceJson, "position");
        instance->position = glm::vec3(cJSON_GetArrayItem(position, 0)->valuedouble, cJSON_GetArrayItem(position, 1)->valuedouble, cJSON_GetArrayItem(position, 2)->valuedouble);

        cJSON *orientation = cJSON_GetObjectItem(instanceJson, "orientation");
        instance->orientation = glm::vec3(cJSON_GetArrayItem(orientation, 0)->valuedouble, cJSON_GetArrayItem(orientation, 1)->valuedouble, cJSON_GetArrayItem(orientation, 2)->valuedouble);

        cJSON *scale = cJSON_GetObjectItem(instanceJson, "scale");
        instance->scale = glm::vec3(cJSON_GetArrayItem(scale, 0)->valuedouble, cJSON_GetArrayItem(scale, 1)->valuedouble, cJSON_GetArrayItem(scale, 2)->valuedouble);

        cJSON *animation = cJSON_GetObjectItem(instanceJson, "animation");
        if (animation)
        {
            PropertyMapping properties;
            properties.add("location", (float *)&instance->position);
            properties.add("rotation_euler", (float *)&instance->orientation);
            properties.add("scale", (float *)&instance->scale);

            instance->animation = new AnimationData(animation, properties);
            this->animationPlayer.registerAnimation(instance->animation);
        }

        this->instances.push_back(instance);

        instanceJson = instanceJson->next;
    }
}

void Scene::unload()
{
    std::for_each(this->instances.begin(), this->instances.end(), [this](MeshInstance *instance)
    {
        ResourceManager::getInstance()->releaseResource(instance->mesh);

        if (instance->animation)
        {
            this->animationPlayer.unregisterAnimation(instance->animation);
            delete instance->animation;
        }

        delete instance;
    });
    this->instances.clear();
}

void Scene::updateAnimation(float time)
{
    this->animationPlayer.update(time);
    AnimationPlayer::globalPlayer.update(time);
}

void Scene::fillRenderList(RenderList *renderList) const
{
    std::for_each(this->instances.begin(), this->instances.end(), [&](const MeshInstance *instance)
    {
        RenderList::Job job;
        job.mesh = instance->mesh;
        job.transform = glm::translate(glm::eulerAngleXYZ(instance->orientation.x, instance->orientation.y, instance->orientation.z) * glm::scale(glm::mat4(), instance->scale), instance->position);
        job.material = instance->mesh->getMaterial();
        renderList->addJob(job);
    });
}
