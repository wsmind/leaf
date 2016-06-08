#include <engine/Scene.h>

#include <cassert>
#include <engine/glm/gtc/matrix_transform.hpp>

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
        cJSON *mat = cJSON_GetObjectItem(instanceJson, "transform");

        MeshInstance *instance = new MeshInstance;
        instance->mesh = ResourceManager::getInstance()->requestResource<Mesh>(meshName);
        instance->transform = glm::mat4(
            cJSON_GetArrayItem(mat, 0)->valuedouble, cJSON_GetArrayItem(mat, 1)->valuedouble, cJSON_GetArrayItem(mat, 2)->valuedouble, cJSON_GetArrayItem(mat, 3)->valuedouble,
            cJSON_GetArrayItem(mat, 4)->valuedouble, cJSON_GetArrayItem(mat, 5)->valuedouble, cJSON_GetArrayItem(mat, 6)->valuedouble, cJSON_GetArrayItem(mat, 7)->valuedouble,
            cJSON_GetArrayItem(mat, 8)->valuedouble, cJSON_GetArrayItem(mat, 9)->valuedouble, cJSON_GetArrayItem(mat, 10)->valuedouble, cJSON_GetArrayItem(mat, 11)->valuedouble,
            cJSON_GetArrayItem(mat, 12)->valuedouble, cJSON_GetArrayItem(mat, 13)->valuedouble, cJSON_GetArrayItem(mat, 14)->valuedouble, cJSON_GetArrayItem(mat, 15)->valuedouble
        );

        cJSON *animation = cJSON_GetObjectItem(instanceJson, "animation");
        if (animation)
        {
            PropertyMapping properties;
            properties.add("location", (float *)&instance->position);

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
        job.transform = glm::translate(glm::mat4(), instance->position);
        job.material = instance->mesh->getMaterial();
        renderList->addJob(job);
    });
}
