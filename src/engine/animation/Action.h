#pragma once

#include <string>
#include <vector>

#include <engine/resource/Resource.h>

class FCurve;
class PropertyMapping;

class Action: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        void evaluate(float time, const PropertyMapping *properties) const;

    private:
        std::vector<FCurve *> curves;
};
