#pragma once

#include <vector>

#include <engine/glm/glm.hpp>

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
        void update(float time, const glm::mat4 &emitterTransform);

        // send active particles as render jobs
        void fillRenderList(RenderList *renderList) const;

    private:
        void createSimulation();
        void destroySimulation();

        void stepSimulation(float deltaTime, const glm::mat4 &emitterTransform);
        
        ParticleSettings *settings;
        int seed;

        float simulationTime;

        struct Particle
        {
            float startTime;
            float endTime;

            bool visible;

            glm::vec3 position; // local to emitter before being born, then world space
            glm::vec3 velocity;

            float size;
        };
        std::vector<Particle> particles;
};
