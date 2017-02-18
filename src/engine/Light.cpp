#include <engine/Light.h>

#include <engine/cJSON/cJSON.h>
#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/PropertyMapping.h>

const std::string Light::resourceClassName = "Light";
const std::string Light::defaultResourceData = "{\"color\": [1.0, 1.0, 1.0], \"radius\": 1.0}";

void Light::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *colorJson = cJSON_GetObjectItem(json, "color");
    this->color = glm::vec3(cJSON_GetArrayItem(colorJson, 0)->valuedouble, cJSON_GetArrayItem(colorJson, 1)->valuedouble, cJSON_GetArrayItem(colorJson, 2)->valuedouble);
    this->radius = (float)cJSON_GetObjectItem(json, "radius")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }

    cJSON_Delete(json);
}

void Light::unload()
{
    if (this->animation)
    {
        AnimationPlayer::globalPlayer.unregisterAnimation(this->animation);
        delete this->animation;
        this->animation = nullptr;
    }
}
