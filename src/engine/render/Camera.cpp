#include <engine/render/Camera.h>

#include <cJSON/cJSON.h>
#include <engine/glm/gtc/matrix_transform.hpp>

#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>

#include <engine/render/RenderSettings.h>

const std::string Camera::resourceClassName = "Camera";
const std::string Camera::defaultResourceData = "{\"lens\": 2.0, \"ortho_scale\": 1.0, \"clip_start\": 0.1, \"clip_end\": 100.0, \"dof_blades\": 6.0, \"dof_distance\": 1.0, \"dof_fstop\": 16.0, \"sensor_height\": 35.0, \"type\": 0, \"shutter_speed\": 0.01}";

void Camera::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    this->lens = (float)cJSON_GetObjectItem(json, "lens")->valuedouble;
    this->ortho_scale = (float)cJSON_GetObjectItem(json, "ortho_scale")->valuedouble;
    this->clipStart = (float)cJSON_GetObjectItem(json, "clip_start")->valuedouble;
    this->clipEnd = (float)cJSON_GetObjectItem(json, "clip_end")->valuedouble;
	this->lensBlades = (float)cJSON_GetObjectItem(json, "dof_blades")->valuedouble;
	this->focusDistance = (float)cJSON_GetObjectItem(json, "dof_distance")->valuedouble;
	this->fstop = (float)cJSON_GetObjectItem(json, "dof_fstop")->valuedouble;
	this->sensorHeight = (float)cJSON_GetObjectItem(json, "sensor_height")->valuedouble;
    this->type = (float)cJSON_GetObjectItem(json, "type")->valuedouble;
    this->shutterSpeed = (float)cJSON_GetObjectItem(json, "shutter_speed")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("lens", &this->lens);
        properties.add("ortho_scale", &this->ortho_scale);
        properties.add("clip_start", &this->clipStart);
        properties.add("clip_end", &this->clipEnd);
		properties.add("gpu_dof.blades", &this->lensBlades);
		properties.add("dof_distance", &this->focusDistance);
		properties.add("gpu_dof.fstop", &this->fstop);
		properties.add("type", &this->type);
        properties.add("leaf.shutter_speed", &this->shutterSpeed);

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }

    cJSON_Delete(json);
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

void Camera::updateSettings(CameraSettings &settings, float aspect)
{
	this->computeProjectionMatrix(settings.projectionMatrix, aspect);
	settings.shutterSpeed = this->shutterSpeed;
	settings.focusDistance = this->focusDistance;
	settings.focalLength = this->lens;
	settings.fstop = this->fstop;
}

void Camera::computeProjectionMatrix(glm::mat4 &projectionMatrix, float aspect) const
{
    if (this->type == 0.0) // PERSP
    {
        // assume vertical sensor fit
        float fovy = 2.0f * atanf(0.5f * this->sensorHeight / this->lens);
        //printf("fovy: %f\n", fovy * 3.1415926535f / 180.0f);

        projectionMatrix = glm::perspective(fovy, aspect, this->clipStart, this->clipEnd);
    }
    else if (this->type == 1.0) // ORTHO
    {
        // assume vertical sensor fit
        float scalex = this->ortho_scale * 0.5f;
        float scaley = scalex / aspect;
        projectionMatrix = glm::ortho(-scalex, scalex, -scaley, scaley, this->clipStart, this->clipEnd);
    }
    else if (this->type == 2.0) // PANO
    {
        // not supported right now
    }
}
