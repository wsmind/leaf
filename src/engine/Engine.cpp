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

void Engine::loadData(cJSON *json)
{
    this->loadDataCollection<Action>(json, "actions");
    this->loadDataCollection<Light>(json, "lights");
    this->loadDataCollection<Camera>(json, "cameras");
    this->loadDataCollection<Image>(json, "images");
    this->loadDataCollection<Texture>(json, "textures");
    this->loadDataCollection<Material>(json, "materials");
    this->loadDataCollection<Mesh>(json, "meshes");
    this->loadDataCollection<Scene>(json, "scenes");
}

void Engine::registerBlob(const std::string &name, const void *buffer)
{
    ResourceManager::getInstance()->registerBlob(name, buffer);
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

    this->renderer->render(this->scene, width, height, false, glm::mat4(), glm::mat4());
}

void Engine::renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    this->renderer->renderBlenderViewport(this->scene, width, height, viewMatrix, projectionMatrix);
}
