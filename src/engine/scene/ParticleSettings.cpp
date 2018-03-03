#include <engine/scene/ParticleSettings.h>

#include <engine/cJSON/cJSON.h>

#include <engine/resource/ResourceManager.h>
#include <engine/render/Mesh.h>

const std::string ParticleSettings::resourceClassName = "ParticleSettings";
const std::string ParticleSettings::defaultResourceData = "{\"count\": 0, \"frame_start\": 0.0, \"frame_end\": 10.0, \"lifetime\": 20.0, \"lifetime_random\": 0.5, \"size\": 1.0, \"size_random\": 0.5, \"duplicate\": \"default\"}";

void ParticleSettings::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    this->count = cJSON_GetObjectItem(json, "count")->valueint;

    this->frameStart =(float) cJSON_GetObjectItem(json, "frame_start")->valuedouble;
    this->frameEnd = (float)cJSON_GetObjectItem(json, "frame_end")->valuedouble;

    this->lifetime = (float)cJSON_GetObjectItem(json, "lifetime")->valuedouble;
    this->lifetimeRandom = (float)cJSON_GetObjectItem(json, "lifetime_random")->valuedouble;

    this->size = (float)cJSON_GetObjectItem(json, "size")->valuedouble;
    this->sizeRandom = (float)cJSON_GetObjectItem(json, "size_random")->valuedouble;

    std::string duplicateName = cJSON_GetObjectItem(json, "duplicate")->valuestring;
    this->duplicate = ResourceManager::getInstance()->requestResource<Mesh>(duplicateName);
}

void ParticleSettings::unload()
{
    ResourceManager::getInstance()->releaseResource(this->duplicate);
}
