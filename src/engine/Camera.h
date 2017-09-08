#pragma once

#include <string>

#include <engine/glm/glm.hpp>
#include <engine/Resource.h>

class AnimationData;

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

        void computeProjectionMatrix(glm::mat4 &projectionMatrix, float aspect) const;

        float getShutterSpeed() const { return this->shutterSpeed; }
    
    private:
        AnimationData *animation = nullptr;

        float lens; // mm
        float ortho_scale;
        float clipStart;
        float clipEnd;
        float sensorHeight;
        float type; // 0 = PERSP, 1 = ORTHO, 2 = PANO
        float shutterSpeed;
};
