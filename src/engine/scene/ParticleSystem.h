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

        // rebuild simulation when the settings change
        virtual void onResourceUpdated(Resource *resource) override;

        // step simulation
        void update(float time);

        // send active particles as render jobs
        void fillRenderList(RenderList *renderList) const;

    private:
        void createSimulation();
        void destroySimulation();

        void updateSimulation(float step);
        
        ParticleSettings *settings;
        int seed;

        float currentTime;
};
