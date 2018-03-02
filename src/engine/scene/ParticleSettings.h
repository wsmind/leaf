#pragma once

#include <string>

#include <engine/resource/Resource.h>

class Mesh;

struct ParticleSettings : public Resource
{
    static const std::string resourceClassName;
    static const std::string defaultResourceData;

    virtual void load(const unsigned char *buffer, size_t size) override;
    virtual void unload() override;

    int count;

    float frameStart;
    float frameEnd;

    float lifetime;
    float lifetimeRandom;

    Mesh *duplicate;
};
