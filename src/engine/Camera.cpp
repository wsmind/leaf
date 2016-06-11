#include <engine/Camera.h>

#include <engine/cJSON/cJSON.h>
#include <engine/glm/gtc/matrix_transform.hpp>

#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/PropertyMapping.h>

const std::string Camera::resourceClassName = "Camera";
const std::string Camera::defaultResourceData = "{}";

void Camera::load(const cJSON *json)
{
    this->lens = (float)cJSON_GetObjectItem(json, "lens")->valuedouble;
    this->clipStart = (float)cJSON_GetObjectItem(json, "clip_start")->valuedouble;
    this->clipEnd = (float)cJSON_GetObjectItem(json, "clip_end")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("lens", &this->lens);

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

void Camera::computeProjectionMatrix(glm::mat4 &projectionMatrix, float aspect) const
{
    // assume a 35mm film size
    float fovy = 2.0f * atanf(35.0f / (2.0f * this->lens));
    printf("fovy: %f\n", fovy);

    projectionMatrix = glm::perspective(fovy, aspect, this->clipStart, this->clipEnd);
}
