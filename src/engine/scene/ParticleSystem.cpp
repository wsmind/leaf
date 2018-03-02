#include <engine/scene/ParticleSystem.h>

#include <engine/cJSON/cJSON.h>

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

void ParticleSystem::fillRenderList(RenderList *renderList) const
{
    Mesh *mesh = this->settings->duplicate;

    for (auto &subMesh : mesh->getSubMeshes())
    {
        RenderList::Job job;
        job.subMesh = &subMesh;
        job.transform = glm::mat4();
        job.previousFrameTransform = glm::mat4();
        job.material = subMesh.material;

        renderList->addJob(job);
    }
}

void ParticleSystem::createSimulation()
{
}

void ParticleSystem::destroySimulation()
{
}
