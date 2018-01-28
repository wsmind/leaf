#include <engine/scene/Scene.h>

#include <cassert>

#include <engine/animation/AnimationData.h>
#include <engine/render/Texture.h>
#include <engine/render/RenderList.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>
#include <engine/glm/gtc/matrix_transform.hpp>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"activeCamera\": 0, \"nodes\": [], \"markers\": [], \"ambientColor\": [0.0, 0.0, 0.0], \"mist\": 0.0, \"environmentMap\": \"__default\", \"bloom\": {\"threshold\": 1.0, \"intensity\": 1.0, \"debug\": false}}";

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

    cJSON *ambientJson = cJSON_GetObjectItem(json, "ambientColor");
    this->renderSettings.environment.ambientColor = glm::vec3(cJSON_GetArrayItem(ambientJson, 0)->valuedouble, cJSON_GetArrayItem(ambientJson, 1)->valuedouble, cJSON_GetArrayItem(ambientJson, 2)->valuedouble);
    this->renderSettings.environment.mist = (float)cJSON_GetObjectItem(json, "mist")->valuedouble;
    this->renderSettings.environment.environmentMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "environmentMap")->valuestring);

	cJSON *bloomJson = cJSON_GetObjectItem(json, "bloom");
	this->renderSettings.bloom.threshold = (float)cJSON_GetObjectItem(bloomJson, "threshold")->valuedouble;
	this->renderSettings.bloom.intensity = (float)cJSON_GetObjectItem(bloomJson, "intensity")->valuedouble;
	this->renderSettings.bloom.debug = (cJSON_GetObjectItem(bloomJson, "debug")->type == cJSON_True);

    cJSON_Delete(json);

    // update current and last frame's transforms
    this->updateTransforms();
    this->updateTransforms();
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

    ResourceManager::getInstance()->releaseResource(this->renderSettings.environment.environmentMap);
}

void Scene::updateAnimation(float time)
{
    this->animationPlayer.update(time);
    AnimationPlayer::globalPlayer.update(time);

    this->currentCamera = findCurrentCamera(time);
}

void Scene::updateTransforms()
{
    // nodes are sorted at export by parenting depth, ensuring
    // correctness in the hierarchy transforms
    std::for_each(this->nodes.begin(), this->nodes.end(), [this](SceneNode *node)
    {
        node->updateTransforms();
    });
}

const RenderSettings &Scene::updateRenderSettings(int width, int height, bool overrideCamera, const glm::mat4 &viewMatrixOverride, const glm::mat4 &projectionMatrixOverride)
{
	this->renderSettings.frameWidth = width;
	this->renderSettings.frameHeight = height;

	float aspect = (float)width / (float)height;
	this->updateCameraSettings(overrideCamera, viewMatrixOverride, projectionMatrixOverride, aspect);

	return this->renderSettings;
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
            job.transform = node->getCurrentTransform();
            job.previousFrameTransform = node->getPreviousFrameTransform();
            job.material = mesh->getMaterial();
            renderList->addJob(job);
        }
    });

    std::for_each(this->lightNodes.begin(), this->lightNodes.end(), [&](const SceneNode *node)
    {
        if (!node->isHidden())
        {
            Light *light = node->getData<Light>();
            glm::mat4 transform = node->getCurrentTransform();

            RenderList::Light renderLight;
            renderLight.position = glm::vec3(transform[3]);
            renderLight.radius = light->getRadius();
            renderLight.color = light->getColor();

            renderLight.spot = (light->getType() == Light::Spot);
            if (renderLight.spot)
            {
                renderLight.spot = true;
                renderLight.direction = glm::vec3(-transform[2]);
                renderLight.angle = light->getSpotAngle();
                renderLight.blend = light->getSpotBlend();
                renderLight.scattering = light->getScattering();

                glm::mat4 viewMatrix = glm::inverse(node->computeViewTransform());
                glm::mat4 projectionMatrix = glm::perspective(light->getSpotAngle(), 1.0f, 0.1f, light->getRadius());
                renderLight.shadowTransform = projectionMatrix * viewMatrix;
            }

            renderList->addLight(renderLight);
        }
    });
}

void Scene::updateCameraSettings(bool overrideCamera, const glm::mat4 &viewMatrixOverride, const glm::mat4 &projectionMatrixOverride, float aspect)
{
	CameraSettings &settings = this->renderSettings.camera;

	if (overrideCamera || (this->currentCamera >= (int)this->nodes.size()))
	{
		// use the provided camera parameters (or default)
		settings.viewMatrix = viewMatrixOverride;
		settings.projectionMatrix = projectionMatrixOverride;
		settings.shutterSpeed = 0.01f; // hardcoded shutter speed for edition camera
		settings.focusDistance = 1.0f;
		settings.focalLength = 35.0f;
		settings.fstop = 16.0f;
	}
	else
	{
		// get camera from scene
		SceneNode *node = this->nodes[this->currentCamera];
		settings.viewMatrix = glm::inverse(node->computeViewTransform());

		node->getData<Camera>()->updateSettings(settings, aspect);
	}
}

int Scene::findCurrentCamera(float time)
{
    if (this->markers.size() == 0)
        return this->activeCamera;

    if (time <= this->markers[0].time)
        return this->markers[0].cameraIndex;

    int index = 1;
    while ((index < (int)this->markers.size()) && (time >= this->markers[index].time))
        index++;

    return this->markers[index - 1].cameraIndex;
}
