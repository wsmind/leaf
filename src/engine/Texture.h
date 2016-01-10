#pragma once

#include <string>

#include <engine/Resource.h>

class Texture: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

    private:
};
