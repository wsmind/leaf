#include <engine/AnimationData.h>

#include <engine/Action.h>
#include <engine/ResourceManager.h>

AnimationData::AnimationData(const cJSON *json)
{
    cJSON *actionName = cJSON_GetObjectItem(json, "action");
    this->action = ResourceManager::getInstance()->requestResource<Action>(actionName->valuestring);
}

AnimationData::~AnimationData()
{
    ResourceManager::getInstance()->releaseResource(this->action);
}
