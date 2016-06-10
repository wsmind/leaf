#include <engine/Scene.h>

#include <cassert>
#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtx/euler_angles.hpp>

#include <engine/AnimationData.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"meshes\": [], \"lights\": []}";

void Scene::load(const cJSON *json)
{
    cJSON *meshesJson = cJSON_GetObjectItem(json, "meshes");
    cJSON *nodeJson = meshesJson->child;
    while (nodeJson)
    {
        SceneNode<Mesh> *node = new SceneNode<Mesh>(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->meshNodes.push_back(node);

        nodeJson = nodeJson->next;
    }
}

void Scene::unload()
{
    std::for_each(this->meshNodes.begin(), this->meshNodes.end(), [this](SceneNode<Mesh> *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });
    this->meshNodes.clear();
}

void Scene::updateAnimation(float time)
{
    this->animationPlayer.update(time);
    AnimationPlayer::globalPlayer.update(time);
}

void Scene::fillRenderList(RenderList *renderList) const
{
    std::for_each(this->meshNodes.begin(), this->meshNodes.end(), [&](const SceneNode<Mesh> *node)
    {
        Mesh *mesh = node->getData();

        RenderList::Job job;
        job.mesh = mesh;
        job.transform = node->computeTransformMatrix();
        job.material = mesh->getMaterial();
        renderList->addJob(job);
    });
}
