#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <mmsystem.h>

#include <engine/Demo.h>
#include <engine/animation/Action.h>
#include <engine/render/Camera.h>
#include <engine/render/Image.h>
#include <engine/render/Material.h>
#include <engine/render/Mesh.h>
#include <engine/render/Renderer.h>
#include <engine/render/Text.h>
#include <engine/render/Texture.h>
#include <engine/resource/ResourceManager.h>
#include <engine/scene/ParticleSettings.h>
#include <engine/scene/Scene.h>

Engine *Engine::instance = nullptr;

void Engine::initialize(int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename, const std::string &shaderPath)
{
    printf("LeafEngine started\n");

    ResourceManager::create();

    // hide window when capturing
    this->hwnd = CreateWindow("static", "Leaf", SS_BITMAP | WS_POPUP | (capture ? 0 : WS_VISIBLE), 0, 0, backbufferWidth, backbufferHeight, NULL, NULL, NULL, 0);

    this->renderer = new Renderer(hwnd, backbufferWidth, backbufferHeight, capture, profileFilename, shaderPath);
    this->demo = ResourceManager::getInstance()->requestResource<Demo>("demo");
}

void Engine::shutdown()
{
    printf("LeafEngine stopped\n");

    ResourceManager::getInstance()->releaseResource(this->demo);

    delete this->renderer;
    this->renderer = nullptr;

    DestroyWindow(this->hwnd);

    ResourceManager::destroy();
}

void Engine::loadData(const void *buffer, size_t size, ExportList *exportList)
{
    const unsigned char *readPosition = (const unsigned char *)buffer;
    const unsigned char *bufferEnd = readPosition + size;

    while (readPosition < bufferEnd)
    {
        unsigned int typeNameSize = *(unsigned int *)readPosition;
        readPosition += sizeof(unsigned int);

        std::string typeName((const char *)readPosition, typeNameSize);
        readPosition += typeNameSize;

        unsigned int resourceNameSize = *(unsigned int *)readPosition;
        readPosition += sizeof(unsigned int);

        std::string resourceName((const char *)readPosition, resourceNameSize);
        readPosition += resourceNameSize;

        unsigned int blobSize = *(unsigned int *)readPosition;
        readPosition += sizeof(unsigned int);

        printf("Loading resource %s (%s), %d bytes\n", resourceName.c_str(), typeName.c_str(), blobSize);

        if (typeName == "Action") ResourceManager::getInstance()->updateResourceData<Action>(resourceName, readPosition, blobSize);
        if (typeName == "Light") ResourceManager::getInstance()->updateResourceData<Light>(resourceName, readPosition, blobSize);
        if (typeName == "Camera") ResourceManager::getInstance()->updateResourceData<Camera>(resourceName, readPosition, blobSize);
        if (typeName == "Image") ResourceManager::getInstance()->updateResourceData<Image>(resourceName, readPosition, blobSize);
        if (typeName == "Texture") ResourceManager::getInstance()->updateResourceData<Texture>(resourceName, readPosition, blobSize);
        if (typeName == "Material") ResourceManager::getInstance()->updateResourceData<Material>(resourceName, readPosition, blobSize);
        if (typeName == "Mesh") ResourceManager::getInstance()->updateResourceData<Mesh>(resourceName, readPosition, blobSize);
        if (typeName == "ParticleSettings") ResourceManager::getInstance()->updateResourceData<ParticleSettings>(resourceName, readPosition, blobSize);
        if (typeName == "Text") ResourceManager::getInstance()->updateResourceData<Text>(resourceName, readPosition, blobSize);
        if (typeName == "Scene") ResourceManager::getInstance()->updateResourceData<Scene>(resourceName, readPosition, blobSize);
        if (typeName == "Demo") ResourceManager::getInstance()->updateResourceData<Demo>(resourceName, readPosition, blobSize);

        if (exportList != nullptr)
        {
            if (typeName == "Material") exportList->materials.push_back(ResourceManager::getInstance()->requestResource<Material>(resourceName));
            if (typeName == "Text") exportList->texts.push_back(ResourceManager::getInstance()->requestResource<Text>(resourceName));
        }

        readPosition += blobSize;
    }
}

int Engine::exportData(const void *buffer, size_t size, const std::string exportPath)
{
    ExportList exportList;
    this->loadData(buffer, size, &exportList);

    int result = this->renderer->exportShaders(exportPath, exportList.materials, exportList.texts);

    for (Material *material : exportList.materials)
        ResourceManager::getInstance()->releaseResource(material);

    for (Text *text : exportList.texts)
        ResourceManager::getInstance()->releaseResource(text);

    return result;
}

void Engine::update(float time)
{
    this->currentTime = time;

    ResourceManager::getInstance()->update();

    Scene *scene = Scene::findCurrentScene(time);
    if (!scene)
        return;

    scene->update(time);
}

void Engine::render(int width, int height, float deltaTime)
{
    // process window events to avoid the window turning unresponsive
    MSG msg;
    while (PeekMessage(&msg, this->hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Scene *scene = Scene::findCurrentScene(this->currentTime);
    if (!scene)
        return;

    const RenderSettings &renderSettings = scene->updateRenderSettings(width, height);
    this->renderer->render(scene, renderSettings, deltaTime);
}

void Engine::renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    Scene *scene = Scene::findCurrentScene(this->currentTime);
    if (!scene)
        return;

    const float fixedDeltaTime = 1.0f / 60.0f;
    const float fps = 60.0f; /* hardcoded 60 fps */

    // update twice to improve motion blur
    float time = this->currentTime;
    this->update(time - fixedDeltaTime * fps);
    this->update(time);

    const RenderSettings &renderSettings = scene->updateRenderSettings(width, height, true, viewMatrix, projectionMatrix);
	this->renderer->renderBlenderViewport(scene, renderSettings);
}

void Engine::renderBlenderFrame(const char *sceneName, int width, int height, float *outputBuffer, float time)
{
    Scene *renderScene = ResourceManager::getInstance()->requestResource<Scene>(sceneName);

    const float fixedDeltaTime = 1.0f / 60.0f;
    const float fps = 60.0f; /* hardcoded 60 fps */

    // render twice to ensure correct motion blur
    this->update(time - fixedDeltaTime * fps);
    const RenderSettings &renderSettings = renderScene->updateRenderSettings(width, height);
    this->renderer->render(renderScene, renderSettings, fixedDeltaTime);

    this->update(time);
    const RenderSettings &renderSettings2 = renderScene->updateRenderSettings(width, height);
    this->renderer->renderBlenderFrame(renderScene, renderSettings2, outputBuffer, fixedDeltaTime);

    ResourceManager::getInstance()->releaseResource(renderScene);
}
