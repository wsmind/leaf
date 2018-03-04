#include <engine/scene/ParticleSystem.h>

#include <engine/cJSON/cJSON.h>

#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtc/random.hpp>

#include <engine/render/Mesh.h>
#include <engine/render/RenderList.h>
#include <engine/scene/ParticleSettings.h>
#include <engine/resource/ResourceManager.h>

ParticleSystem::ParticleSystem(const cJSON *json)
{
    this->settings = ResourceManager::getInstance()->requestResource<ParticleSettings>(cJSON_GetObjectItem(json, "settings")->valuestring, this);
    this->seed = cJSON_GetObjectItem(json, "seed")->valueint;

    this->createSimulation();
}

ParticleSystem::~ParticleSystem()
{
    this->destroySimulation();
    ResourceManager::getInstance()->releaseResource(this->settings, this);
}

void ParticleSystem::onResourceUpdated(Resource *resource)
{
    // just reinitialize the whole simulation if parameters change
    this->destroySimulation();
    this->createSimulation();
}

void ParticleSystem::update(float time, const glm::mat4 &emitterTransform)
{
    // most basic simulation stepping ever
    this->stepSimulation(time - this->simulationTime, emitterTransform);
}

void ParticleSystem::fillRenderList(RenderList *renderList) const
{
    Mesh *mesh = this->settings->duplicate;

    for (auto &subMesh : mesh->getSubMeshes())
    {
        RenderList::Job job;
        job.subMesh = &subMesh;
        job.material = subMesh.material;

        for (const auto &particle : this->particles)
        {
            if (!particle.visible)
                continue;

            glm::mat4 transform = glm::translate(glm::mat4(), particle.position) * glm::scale(glm::mat4(), glm::vec3(particle.size));
            job.transform = transform;
            job.previousFrameTransform = transform;
            renderList->addJob(job);
        }
    }
}

void ParticleSystem::createSimulation()
{
    this->simulationTime = 0.0f;

    this->particles.resize(this->settings->count);
    for (int i = 0; i < this->settings->count; i++)
    {
        Particle &particle = this->particles[i];

        // spread all particles linearly between start and end times
        particle.startTime = glm::mix(this->settings->frameStart, this->settings->frameEnd, (float)i / (float)this->settings->count);

        float lifetime = this->settings->lifetime * (1.0f - glm::linearRand(0.0f, this->settings->lifetimeRandom));
        particle.endTime = particle.startTime + lifetime;

        particle.visible = false;
        particle.spawnPosition = glm::ballRand(1.0f);
        particle.spawnVelocity = glm::ballRand(0.04f);

        // cubify initial position
        particle.spawnPosition /= max(max(abs(particle.spawnPosition.x), abs(particle.spawnPosition.y)), abs(particle.spawnPosition.z));

        particle.size = this->settings->size * (1.0f - glm::linearRand(0.0f, this->settings->sizeRandom));
    }
}

void ParticleSystem::destroySimulation()
{
    this->particles.clear();
}

void ParticleSystem::stepSimulation(float deltaTime, const glm::mat4 &emitterTransform)
{
    this->simulationTime += deltaTime;

    for (auto &particle : this->particles)
    {
        bool unborn = this->simulationTime < particle.startTime;
        bool dead = this->simulationTime > particle.endTime;
        bool alive = !unborn && !dead;

        particle.visible = alive;

        if (alive)
        {
            // basic euler integration
            particle.velocity.z -= 9.81f * 0.0001f * deltaTime;
            particle.position += particle.velocity * deltaTime;
        }
        else if (unborn)
        {
            // move with emitter
            particle.position = glm::vec3(emitterTransform * glm::vec4(particle.spawnPosition, 1.0f));
            particle.velocity = glm::mat3(emitterTransform) * particle.spawnVelocity;
        }
    }
}
