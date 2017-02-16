#include <engine/Scene.h>

#include <cassert>

#include <engine/AnimationData.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"activeCamera\": 0, \"nodes\": [], \"markers\": []}";

void Scene::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    this->activeCamera = cJSON_GetObjectItem(json, "activeCamera")->valueint;

    cJSON *nodesJson = cJSON_GetObjectItem(json, "nodes");
    cJSON *nodeJson = nodesJson->child;
    while (nodeJson)
    {
        // resolve parent pointer
        cJSON *parentIndex = cJSON_GetObjectItem(nodeJson, "parent");
        SceneNode *parent = nullptr;
        if (parentIndex != nullptr)
            parent = this->nodes[parentIndex->valueint];

        SceneNode *node = new SceneNode(nodeJson, parent);
        node->registerAnimation(&this->animationPlayer);

        this->nodes.push_back(node);

        int type = cJSON_GetObjectItem(nodeJson, "type")->valueint;
        switch (type)
        {
            case 0: this->cameraNodes.push_back(node); break;
            case 1: this->meshNodes.push_back(node); break;
            case 2: this->lightNodes.push_back(node); break;
        }

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

    cJSON_Delete(json);
}

void Scene::unload()
{
    std::for_each(this->nodes.begin(), this->nodes.end(), [this](SceneNode *node)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    });

    this->nodes.clear();

    this->cameraNodes.clear();
    this->meshNodes.clear();
    this->lightNodes.clear();
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

    std::for_each(this->lightNodes.begin(), this->lightNodes.end(), [&](const SceneNode *node)
    {
        if (!node->isHidden())
        {
            Light *light = node->getData<Light>();

            RenderList::Light renderLight;
            renderLight.position = glm::vec3(node->computeTransformMatrix()[3]);
            renderLight.distance = light->getDistance();
            renderLight.color = light->getColor();
            renderList->addLight(renderLight);
        }
    });
}

void Scene::setupCameraMatrices(glm::mat4 &viewMatrix, glm::mat4 &projectionMatrix, float aspect) const
{
    if (this->currentCamera >= this->nodes.size())
        return;

    SceneNode *node = this->nodes[this->currentCamera];
    Camera *camera = node->getData<Camera>();

    viewMatrix = glm::inverse(node->computeTransformMatrix());
    camera->computeProjectionMatrix(projectionMatrix, aspect);
}

int Scene::findCurrentCamera(float time)
{
    if (this->markers.size() == 0)
        return this->activeCamera;

    if (time <= this->markers[0].time)
        return this->markers[0].cameraIndex;

    int index = 1;
    while ((index < this->markers.size()) && (time >= this->markers[index].time))
        index++;

    return this->markers[index - 1].cameraIndex;
}
