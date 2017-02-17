#pragma once

#include <string>

#include <engine/glm/glm.hpp>
#include <engine/Resource.h>

class AnimationData;

class Light: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        glm::vec3 getColor() const { return this->color; }
        float getRadius() const { return this->radius; }
    
    private:
        glm::vec3 color;
        float radius;
        AnimationData *animation = nullptr;
};
