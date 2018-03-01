#include <engine/scene/ParticleSettings.h>

#include <engine/cJSON/cJSON.h>

const std::string ParticleSettings::resourceClassName = "ParticleSettings";
const std::string ParticleSettings::defaultResourceData = "{\"count\": 0, \"frame_start\": 0.0, \"frame_end\": 10.0, \"lifetime\": 20.0, \"lifetime_random\": 0.5}";

void ParticleSettings::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    this->count = cJSON_GetObjectItem(json, "count")->valueint;

    this->frameStart =(float) cJSON_GetObjectItem(json, "frame_start")->valuedouble;
    this->frameEnd = (float)cJSON_GetObjectItem(json, "frame_end")->valuedouble;

    this->lifetime = (float)cJSON_GetObjectItem(json, "lifetime")->valuedouble;
    this->lifetimeRandom = (float)cJSON_GetObjectItem(json, "lifetime_random")->valuedouble;
}
