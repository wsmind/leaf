#include <engine/scene/ParticleSystem.h>

#include <engine/cJSON/cJSON.h>

#include <engine/scene/ParticleSettings.h>
#include <engine/resource/ResourceManager.h>

ParticleSystem::ParticleSystem(const cJSON *json)
{
    this->settings = ResourceManager::getInstance()->requestResource<ParticleSettings>(cJSON_GetObjectItem(json, "settings")->valuestring, this);
    this->seed = cJSON_GetObjectItem(json, "seed")->valueint;
}

ParticleSystem::~ParticleSystem()
{
    ResourceManager::getInstance()->releaseResource(this->settings, this);
}

void ParticleSystem::onResourceUpdated(Resource *resource)
{
}
