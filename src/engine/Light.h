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

        // this enum is serialized in json data (see export.py)
        enum LightType
        {
            Point = 0,
            Spot = 1
        };

        LightType getType() const { return this->type; }
        glm::vec3 getColor() const { return this->color * this->energy; }
        float getRadius() const { return this->radius; }
        float getSpotAngle() const { return this->spotAngle; }
        float getSpotBlend() const { return this->spotBlend; }

    private:
        LightType type;
        glm::vec3 color;
        float energy;
        float radius;
        float spotAngle;
        float spotBlend;

        AnimationData *animation = nullptr;
};
