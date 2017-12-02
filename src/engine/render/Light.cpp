#include <engine/render/Light.h>

#include <engine/cJSON/cJSON.h>
#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>

const std::string Light::resourceClassName = "Light";
const std::string Light::defaultResourceData = "{\"type\": 0,\"color\": [1.0, 1.0, 1.0], \"energy\": 1.0, \"radius\": 1.0, \"spotAngle\": 3.14, \"spotBlend\": 1.0, \"scattering\": 0.0}";

void Light::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *colorJson = cJSON_GetObjectItem(json, "color");
    this->type = (LightType)cJSON_GetObjectItem(json, "type")->valueint;
    this->color = glm::vec3(cJSON_GetArrayItem(colorJson, 0)->valuedouble, cJSON_GetArrayItem(colorJson, 1)->valuedouble, cJSON_GetArrayItem(colorJson, 2)->valuedouble);
    this->energy = (float)cJSON_GetObjectItem(json, "energy")->valuedouble;
    this->radius = (float)cJSON_GetObjectItem(json, "radius")->valuedouble;
    this->spotAngle = (float)cJSON_GetObjectItem(json, "spotAngle")->valuedouble;
    this->spotBlend = (float)cJSON_GetObjectItem(json, "spotBlend")->valuedouble;
    this->scattering = (float)cJSON_GetObjectItem(json, "scattering")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("color", (float *)&this->color);
        properties.add("energy", &this->energy);
        properties.add("distance", &this->radius);
        properties.add("spot_size", &this->spotAngle);
        properties.add("spot_blend", &this->spotBlend);
        properties.add("leaf.scattering", &this->scattering);

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
