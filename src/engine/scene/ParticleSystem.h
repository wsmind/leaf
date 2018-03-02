#pragma once

#include <engine/resource/ResourceWatcher.h>

struct cJSON;
struct ParticleSettings;

class ParticleSystem: public ResourceWatcher
{
    public:
        ParticleSystem(const cJSON *json);
        ~ParticleSystem();

        // update simulation when the settings change
        virtual void onResourceUpdated(Resource *resource) override;

    private:
        ParticleSettings *settings;
        int seed;
};
