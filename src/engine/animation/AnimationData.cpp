#include <engine/animation/AnimationData.h>

#include <engine/animation/Action.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

AnimationData::AnimationData(const cJSON *json, const PropertyMapping &properties)
{
    cJSON *actionName = cJSON_GetObjectItem(json, "action");
    this->action = ResourceManager::getInstance()->requestResource<Action>(actionName->valuestring);

    this->properties = properties;
}

AnimationData::~AnimationData()
{
    ResourceManager::getInstance()->releaseResource(this->action);
}

void AnimationData::update(float time)
{
    this->action->evaluate(time, &this->properties);
}
