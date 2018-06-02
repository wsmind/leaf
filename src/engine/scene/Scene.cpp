#include <engine/scene/Scene.h>

#include <cassert>

#include <engine/animation/AnimationData.h>
#include <engine/render/Texture.h>
#include <engine/render/RenderList.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>
#include <engine/glm/gtc/matrix_transform.hpp>

const std::string Scene::resourceClassName = "Scene";
const std::string Scene::defaultResourceData = "{\"activeCamera\": 0, \"nodes\": [], \"markers\": [], \"frame_start\": 0.0, \"frame_end\": 10.0, \"ambientColor\": [0.0, 0.0, 0.0], \"mist\": 0.0, \"environmentMap\": \"__default\", \"bloom\": {\"threshold\": 1.0, \"intensity\": 1.0, \"size\": 4, \"debug\": false}, \"postprocess\": {\"pixellate_divider\": 0.0, \"vignette_size\": 1.0, \"vignette_power\": 1.6, \"abberation_strength\": 0.1, \"scanline_strength\": 0.0, \"scanline_frequency\": 20.0, \"scanline_offset\": 0.0}}";

std::vector<Scene *> Scene::allScenes;

void Scene::load(const unsigned char *buffer, size_t size)
{
    this->animation = nullptr;

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

        if (node->hasParticleSystems())
            this->particleSystemNodes.push_back(node);

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

    this->frameStart = (float)cJSON_GetObjectItem(json, "frame_start")->valuedouble;
    this->frameEnd = (float)cJSON_GetObjectItem(json, "frame_end")->valuedouble;

    cJSON *ambientJson = cJSON_GetObjectItem(json, "ambientColor");
    this->renderSettings.environment.ambientColor = glm::vec3(cJSON_GetArrayItem(ambientJson, 0)->valuedouble, cJSON_GetArrayItem(ambientJson, 1)->valuedouble, cJSON_GetArrayItem(ambientJson, 2)->valuedouble);
    this->renderSettings.environment.mist = (float)cJSON_GetObjectItem(json, "mist")->valuedouble;
    this->renderSettings.environment.environmentMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "environmentMap")->valuestring);

	cJSON *bloomJson = cJSON_GetObjectItem(json, "bloom");
	this->renderSettings.bloom.threshold = (float)cJSON_GetObjectItem(bloomJson, "threshold")->valuedouble;
	this->renderSettings.bloom.intensity = (float)cJSON_GetObjectItem(bloomJson, "intensity")->valuedouble;
    this->renderSettings.bloom.size = (float)cJSON_GetObjectItem(bloomJson, "size")->valuedouble;
    this->renderSettings.bloom.debug = (cJSON_GetObjectItem(bloomJson, "debug")->type == cJSON_True);

    cJSON *pixellateJson = cJSON_GetObjectItem(json, "postprocess");
    this->renderSettings.postProcess.pixellateDivider = (float)cJSON_GetObjectItem(pixellateJson, "pixellate_divider")->valuedouble;
    this->renderSettings.postProcess.vignetteSize = (float)cJSON_GetObjectItem(pixellateJson, "vignette_size")->valuedouble;
    this->renderSettings.postProcess.vignettePower = (float)cJSON_GetObjectItem(pixellateJson, "vignette_power")->valuedouble;
    this->renderSettings.postProcess.abberationStrength = (float)cJSON_GetObjectItem(pixellateJson, "abberation_strength")->valuedouble;
    this->renderSettings.postProcess.scanlineStrength = (float)cJSON_GetObjectItem(pixellateJson, "scanline_strength")->valuedouble;
    this->renderSettings.postProcess.scanlineFrequency = (float)cJSON_GetObjectItem(pixellateJson, "scanline_frequency")->valuedouble;
    this->renderSettings.postProcess.scanlineOffset = (float)cJSON_GetObjectItem(pixellateJson, "scanline_offset")->valuedouble;

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("leaf.bloom_threshold", (float *)&this->renderSettings.bloom.threshold);
        properties.add("leaf.bloom_intensity", (float *)&this->renderSettings.bloom.intensity);
        properties.add("leaf.bloom_size", (float *)&this->renderSettings.bloom.size);
        properties.add("leaf.pixellate_divider", (float *)&this->renderSettings.postProcess.pixellateDivider);
        properties.add("leaf.vignette_size", (float *)&this->renderSettings.postProcess.vignetteSize);
        properties.add("leaf.vignette_power", (float *)&this->renderSettings.postProcess.vignettePower);
        properties.add("leaf.abberation_strength", (float *)&this->renderSettings.postProcess.abberationStrength);
        properties.add("leaf.scanline_strength", (float *)&this->renderSettings.postProcess.scanlineStrength);
        properties.add("leaf.scanline_frequency", (float *)&this->renderSettings.postProcess.scanlineFrequency);
        properties.add("leaf.scanline_offset", (float *)&this->renderSettings.postProcess.scanlineOffset);

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }

    cJSON_Delete(json);

    Scene::allScenes.push_back(this);
}

void Scene::unload()
{
    if (this->animation)
    {
        AnimationPlayer::globalPlayer.unregisterAnimation(this->animation);
        delete this->animation;
        this->animation = nullptr;
    }

    Scene::allScenes.erase(std::remove(Scene::allScenes.begin(), Scene::allScenes.end(), this), Scene::allScenes.end());

    for (SceneNode *node : this->nodes)
    {
        node->unregisterAnimation(&this->animationPlayer);
        delete node;
    }

    this->nodes.clear();

    this->cameraNodes.clear();
    this->meshNodes.clear();
    this->lightNodes.clear();

    ResourceManager::getInstance()->releaseResource(this->renderSettings.environment.environmentMap);
}

void Scene::update(float time)
{
    this->animationPlayer.update(time);
    AnimationPlayer::globalPlayer.update(time);

    this->currentCamera = findCurrentCamera(time);

    // nodes are sorted at export by parenting depth, ensuring
    // correctness in the hierarchy transforms
    for (SceneNode *node : this->nodes)
    {
        node->updateTransforms();
    }

    // step particle simulations
    for (SceneNode *node : this->particleSystemNodes)
    {
        node->updateParticles(time);
    }
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
    for (const SceneNode *node : this->meshNodes)
    {
        if (!node->isHidden())
        {
            Mesh *mesh = node->getData<Mesh>();

            for (auto &subMesh : mesh->getSubMeshes())
            {
                RenderList::Job job;
                job.subMesh = &subMesh;
                job.transform = node->getCurrentTransform();
                job.previousFrameTransform = node->getPreviousFrameTransform();
                job.material = subMesh.material;

                renderList->addJob(job);
            }
        }
    }

    for (const SceneNode *node : this->particleSystemNodes)
    {
        if (!node->isHidden())
        {
            // append particles attached to this node
            node->fillParticleRenderList(renderList);
        }
    }

    for (const SceneNode *node: this->lightNodes)
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
    }
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

Scene *Scene::findCurrentScene(float time)
{
    for (auto scene : Scene::allScenes)
    {
        if ((time >= scene->frameStart) && (time < scene->frameEnd))
        {
            return scene;
        }
    }

    return nullptr;
}
