#pragma once

#include <string>

#include <engine/Resource.h>

class Material: public Resource
{
    public:
        static const std::string resourceClassName;

        virtual void load(const cJSON *json) override;
        virtual void unload() override;
};
