#pragma once

#include <string>

#include <engine/glm/glm.hpp>
#include <engine/resource/Resource.h>

class AnimationData;
struct CameraSettings;

class Camera: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Camera()
            : lens(35.0f)
            , clipStart(0.1f)
            , clipEnd(100.0f)
            , sensorHeight(32.0f)
        {}

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

		void updateSettings(CameraSettings *settings, float aspect);
    
    private:
		void computeProjectionMatrix(glm::mat4 &projectionMatrix, float aspect) const;

		AnimationData *animation = nullptr;

        float lens; // mm
        float ortho_scale;
        float clipStart;
        float clipEnd;
		float lensBlades;
		float focusDistance;
		float fstop;
        float sensorHeight;
        float type; // 0 = PERSP, 1 = ORTHO, 2 = PANO
        float shutterSpeed;
};
