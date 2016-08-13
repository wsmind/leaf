#include <engine/Scene.h>

#include <cassert>

#include <engine/AnimationData.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"meshes\": [], \"lights\": [], \"cameras\": [], \"markers\": []}";

void Scene::load(const cJSON *json)
{
    cJSON *meshesJson = cJSON_GetObjectItem(json, "meshes");
    cJSON *nodeJson = meshesJson->child;
    while (nodeJson)
    {
        SceneNode*node = new SceneNode(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->meshNodes.push_back(node);

        nodeJson = nodeJson->next;
    }

    cJSON *lightsJson = cJSON_GetObjectItem(json, "lights");
    nodeJson = lightsJson->child;
    while (nodeJson)
    {
        SceneNode *node = new SceneNode(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->lightNodes.push_back(node);

        nodeJson = nodeJson->next;
    }

    cJSON *camerasJson = cJSON_GetObjectItem(json, "cameras");
    nodeJson = camerasJson->child;
    while (nodeJson)
    {
        SceneNode *node = new SceneNode(nodeJson);
        node->registerAnimation(&this->animationPlayer);
        this->cameraNodes.push_back(node);

        nodeJson = nodeJson->next;
    }

    cJSON *markersJson = cJSON_GetObjectItem(json, "markers");
    cJSON *markerJson = markersJson->child;
    while (markerJson)
    {
        Marker marker;
        marker.cameraIndex = cJSON_GetObjectItem(markerJson, "camera")->valueint;
        marker.time = (float)cJSON_GetObjectItem(markerJson, "time")->valuedouble;
        this->markers.push_back(marker);

        markerJson = markerJson->next;
    }
}

void Scene::unload()
{
    std::for_each(this->meshNodes.begin(), this->meshNodes.end(), [this](SceneNode *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });
    this->meshNodes.clear();

    std::for_each(this->lightNodes.begin(), this->lightNodes.end(), [this](SceneNode *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });
    this->lightNodes.clear();

   std::for_each(this->cameraNodes.begin(), this->cameraNodes.end(), [this](SceneNode *node)
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

    this->currentCamera = findCurrentCamera(time);
}

void Scene::fillRenderList(RenderList *renderList) const
{
    std::for_each(this->meshNodes.begin(), this->meshNodes.end(), [&](const SceneNode *node)
    {
        if (!node->isHidden())
        {
            Mesh *mesh = node->getData<Mesh>();

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
    if (this->currentCamera >= this->cameraNodes.size())
        return;

    SceneNode *node = this->cameraNodes[this->currentCamera];
    Camera *camera = node->getData<Camera>();

    viewMatrix = glm::inverse(node->computeTransformMatrix());
    camera->computeProjectionMatrix(projectionMatrix, aspect);
}

int Scene::findCurrentCamera(float time)
{
    if (this->markers.size() == 0)
        return 0;

    if (time <= this->markers[0].time)
        return this->markers[0].cameraIndex;

    int index = 1;
    while ((index < this->markers.size()) && (time >= this->markers[index].time))
        index++;

    return this->markers[index - 1].cameraIndex;
}
