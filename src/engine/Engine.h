#pragma once

#include <cassert>
#include <string>

#include <windows.h>

#include <engine/glm/glm.hpp>

struct cJSON;
class Renderer;
class Scene;

class Engine
{
    public:
        void initialize(int backbufferWidth, int backbufferHeight, bool capture);
        void shutdown();

        void loadData(cJSON *json);
        void registerBlob(const std::string &name, const void *buffer);

        void render(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
        void renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

    private:
        static Engine *instance;

        HWND hwnd;
        DWORD startTime;

        Renderer *renderer;
        Scene *scene;

public:
        // singleton implementation
        static void create() { assert(!Engine::instance); Engine::instance = new Engine; }
        static void destroy() { assert(Engine::instance); delete Engine::instance; }
        static Engine *getInstance() { assert(Engine::instance); return Engine::instance; }
};
