#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <mmsystem.h>

#include <engine/Action.h>
#include <engine/Camera.h>
#include <engine/Image.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/Renderer.h>
#include <engine/ResourceManager.h>
#include <engine/Scene.h>
#include <engine/Texture.h>

Engine *Engine::instance = nullptr;

void Engine::initialize(int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename)
{
    printf("LeafEngine started\n");

    ResourceManager::create();

    // hide window when capturing
    this->hwnd = CreateWindow("static", "Leaf", WS_POPUP | (capture ? 0 : WS_VISIBLE), 0, 0, backbufferWidth, backbufferHeight, NULL, NULL, NULL, 0);

    this->renderer = new Renderer(hwnd, backbufferWidth, backbufferHeight, capture, profileFilename);
    this->scene = ResourceManager::getInstance()->requestResource<Scene>("Scene");
}

void Engine::shutdown()
{
    printf("LeafEngine stopped\n");

    ResourceManager::getInstance()->releaseResource(this->scene);

    delete this->renderer;
    this->renderer = nullptr;

    DestroyWindow(this->hwnd);

    ResourceManager::destroy();
}

void Engine::loadData(const void *buffer, size_t size)
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
        if (typeName == "Scene") ResourceManager::getInstance()->updateResourceData<Scene>(resourceName, readPosition, blobSize);

        readPosition += blobSize;
    }
}

void Engine::updateAnimation(float time)
{
    this->scene->updateAnimation(time);
}

void Engine::render(int width, int height)
{
    // process window events to avoid the window turning unresponsive
    MSG msg;
    while (PeekMessage(&msg, this->hwnd, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ResourceManager::getInstance()->update();

    this->scene->updateTransforms();
    this->renderer->render(this->scene, width, height, false, glm::mat4(), glm::mat4());
}

void Engine::renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    ResourceManager::getInstance()->update();

    this->scene->updateTransforms();
    this->renderer->renderBlenderViewport(this->scene, width, height, viewMatrix, projectionMatrix);
}
