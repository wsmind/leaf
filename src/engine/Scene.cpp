#include <engine/Scene.h>

#include <cassert>
#include <engine/glm/gtc/matrix_transform.hpp>
#include <engine/glm/gtx/euler_angles.hpp>

#include <engine/AnimationData.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"meshes\": [], \"lights\": [], \"cameras\": []}";

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

    cJSON *lightsJson = cJSON_GetObjectItem(json, "lights");
    nodeJson = lightsJson->child;
    while (nodeJson)
    {
        SceneNode<Light> *node = new SceneNode<Light>(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->lightNodes.push_back(node);

        nodeJson = nodeJson->next;
    }

    cJSON *camerasJson = cJSON_GetObjectItem(json, "cameras");
    nodeJson = camerasJson->child;
    while (nodeJson)
    {
        SceneNode<Camera> *node = new SceneNode<Camera>(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->cameraNodes.push_back(node);

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

    std::for_each(this->lightNodes.begin(), this->lightNodes.end(), [this](SceneNode<Light> *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });
    this->lightNodes.clear();

   std::for_each(this->cameraNodes.begin(), this->cameraNodes.end(), [this](SceneNode<Camera> *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });
    this->cameraNodes.clear();
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
        if (!node->isHidden())
        {
            Mesh *mesh = node->getData();

            RenderList::Job job;
            job.mesh = mesh;
            job.transform = node->computeTransformMatrix();
            job.material = mesh->getMaterial();
            renderList->addJob(job);
        }
    });
}

void Scene::setupCameraMatrices(glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, float aspect) const
{
    if (this->cameraNodes.size() == 0)
        return;

    SceneNode<Camera> *node = this->cameraNodes[0];
    Camera *camera = node->getData();

    viewMatrix = glm::inverse(node->computeTransformMatrix());
    camera->computeProjectionMatrix(projectionMatrix, aspect);
}
