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
        {}

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        void computeProjectionMatrix(glm::mat4 &projectionMatrix, float aspect) const;
    
    private:
        AnimationData *animation = nullptr;

        float lens; // mm
        float clipStart;
        float clipEnd;
};
