#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <mmsystem.h>

#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/Renderer.h>
#include <engine/ResourceManager.h>
#include <engine/Scene.h>

Engine *Engine::instance = nullptr;

void Engine::initialize(int backbufferWidth, int backbufferHeight, bool capture)
{
    printf("LeafEngine started\n");

    ResourceManager::create();

    // hide window when capturing
    this->hwnd = CreateWindow("static", "Leaf", WS_POPUP | (capture ? 0 : WS_VISIBLE), 0, 0, backbufferWidth, backbufferHeight, NULL, NULL, NULL, 0);

    this->renderer = new Renderer(hwnd, backbufferWidth, backbufferHeight, capture);
    this->scene = ResourceManager::getInstance()->requestResource<Scene>("Scene");

    startTime = timeGetTime();
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
    cJSON *scenes = cJSON_GetObjectItem(json, "scenes");
    if (scenes)
    {
        cJSON *scene = scenes->child;
        while (scene)
        {
            std::string name = scene->string;
            ResourceManager::getInstance()->updateResourceData<Scene>(name, scene);

            scene = scene->next;
        }
    }

    cJSON *materials = cJSON_GetObjectItem(json, "materials");
    if (materials)
    {
        cJSON *material = materials->child;
        while (material)
        {
            std::string name = material->string;
            ResourceManager::getInstance()->updateResourceData<Material>(name, material);

            material = material->next;
        }
    }

    cJSON *meshes = cJSON_GetObjectItem(json, "meshes");
    if (meshes)
    {
        cJSON *mesh = meshes->child;
        while (mesh)
        {
            std::string name = mesh->string;
            ResourceManager::getInstance()->updateResourceData<Mesh>(name, mesh);

            mesh = mesh->next;
        }
    }
}

void Engine::render(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    float time = (float)(timeGetTime() - startTime) * 0.001f * 140.0f / 60.0f;
    this->renderer->render(nullptr, width, height, viewMatrix, projectionMatrix, time);
}

void Engine::renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    float time = (float)(timeGetTime() - startTime) * 0.001f * 140.0f / 60.0f;
    this->renderer->renderBlenderViewport(nullptr, width, height, viewMatrix, projectionMatrix, time);
}
