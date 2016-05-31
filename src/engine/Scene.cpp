#include <engine/Scene.h>

#include <cassert>
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

        MeshInstance instance;
        instance.mesh = ResourceManager::getInstance()->requestResource<Mesh>(meshName);
        instance.transform = glm::mat4(
            cJSON_GetArrayItem(mat, 0)->valuedouble, cJSON_GetArrayItem(mat, 1)->valuedouble, cJSON_GetArrayItem(mat, 2)->valuedouble, cJSON_GetArrayItem(mat, 3)->valuedouble,
            cJSON_GetArrayItem(mat, 4)->valuedouble, cJSON_GetArrayItem(mat, 5)->valuedouble, cJSON_GetArrayItem(mat, 6)->valuedouble, cJSON_GetArrayItem(mat, 7)->valuedouble,
            cJSON_GetArrayItem(mat, 8)->valuedouble, cJSON_GetArrayItem(mat, 9)->valuedouble, cJSON_GetArrayItem(mat, 10)->valuedouble, cJSON_GetArrayItem(mat, 11)->valuedouble,
            cJSON_GetArrayItem(mat, 12)->valuedouble, cJSON_GetArrayItem(mat, 13)->valuedouble, cJSON_GetArrayItem(mat, 14)->valuedouble, cJSON_GetArrayItem(mat, 15)->valuedouble
        );

        cJSON *animation = cJSON_GetObjectItem(instanceJson, "animation");
        if (animation)
            instance.animation = new AnimationData(animation);

        this->instances.push_back(instance);

        instanceJson = instanceJson->next;
    }
}

void Scene::unload()
{
    std::for_each(this->instances.begin(), this->instances.end(), [](MeshInstance &instance)
    {
        ResourceManager::getInstance()->releaseResource(instance.mesh);
        delete instance.animation;
    });
    this->instances.clear();
}

void Scene::fillRenderList(RenderList *renderList) const
{
    std::for_each(this->instances.begin(), this->instances.end(), [&](const MeshInstance &instance)
    {
        RenderList::Job job;
        job.mesh = instance.mesh;
        job.transform = instance.transform;
        job.material = instance.mesh->getMaterial();
        renderList->addJob(job);
    });
}
