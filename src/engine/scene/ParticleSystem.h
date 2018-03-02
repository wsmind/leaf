#pragma once

#include <engine/resource/ResourceWatcher.h>

struct cJSON;
class Mesh;
struct ParticleSettings;
class RenderList;

class ParticleSystem: public ResourceWatcher
{
    public:
        ParticleSystem(const cJSON *json);
        ~ParticleSystem();

        // update simulation when the settings change
        virtual void onResourceUpdated(Resource *resource) override;

        // send active particles as render jobs
        void fillRenderList(RenderList *renderList) const;

    private:
        void createSimulation();
        void destroySimulation();
        
        ParticleSettings *settings;
        int seed;
};
