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

        virtual void load(const cJSON *json) override;
        virtual void unload() override;
    
    private:
        AnimationData *animation = nullptr;
};
