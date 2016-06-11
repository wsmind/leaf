#include <engine/Light.h>

#include <engine/cJSON/cJSON.h>
#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/PropertyMapping.h>

const std::string Light::resourceClassName = "Light";
const std::string Light::defaultResourceData = "{}";

void Light::load(const cJSON *json)
{
    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }
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
