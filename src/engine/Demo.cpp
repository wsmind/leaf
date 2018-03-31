#include <engine/Demo.h>

#include <cassert>

#include <engine/resource/ResourceManager.h>
#include <engine/scene/Scene.h>

#include <engine/cJSON/cJSON.h>

const std::string Demo::resourceClassName = "Demo";
const std::string Demo::defaultResourceData = "{\"scenes\": []}";

void Demo::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *scenesJson = cJSON_GetObjectItem(json, "scenes");
    cJSON *sceneJson = scenesJson->child;
    while (sceneJson)
    {
        Scene *scene = ResourceManager::getInstance()->requestResource<Scene>(sceneJson->valuestring);
        this->scenes.push_back(scene);

        sceneJson = sceneJson->next;
    }

    cJSON_Delete(json);
}

void Demo::unload()
{
    for (auto scene : this->scenes)
        ResourceManager::getInstance()->releaseResource(scene);
}
