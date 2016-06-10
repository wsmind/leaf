#include <engine/Camera.h>

#include <engine/cJSON/cJSON.h>
#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/PropertyMapping.h>

const std::string Camera::resourceClassName = "Camera";
const std::string Camera::defaultResourceData = "{}";

void Camera::load(const cJSON *json)
{
    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }
}

void Camera::unload()
{
    if (this->animation)
    {
        AnimationPlayer::globalPlayer.unregisterAnimation(this->animation);
        delete this->animation;
        this->animation = nullptr;
    }
}
