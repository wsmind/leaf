#pragma once

#include <vector>
#include <string>

#include <engine/resource/Resource.h>

class Scene;

class Demo : public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Demo() {}

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

    private:
        std::vector<Scene *> scenes;
};
