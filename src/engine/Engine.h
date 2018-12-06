#pragma once

#include <cassert>
#include <string>
#include <vector>

#include <windows.h>

#include <glm/glm.hpp>

struct cJSON;
class Renderer;
class Demo;
class Material;
class Text;

class Engine
{
    public:
        void initialize(int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename, const std::string &shaderPath);
        void shutdown();

        struct ExportList
        {
            std::vector<Material *> materials;
            std::vector<Text *> texts;
        };

        void loadData(const void *buffer, size_t size, ExportList *exportList = nullptr);
        int exportData(const void *buffer, size_t size, const std::string exportPath);

        void update(float time);

        void render(int width, int height, float deltaTime);
        void renderBlenderViewport(int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
        void renderBlenderFrame(const char *sceneName, int width, int height, float *outputBuffer, float time);

    private:
        static Engine *instance;

        HWND hwnd;

        Renderer *renderer;

        Demo *demo;

        float currentTime = 0.0f;

    public:
        // singleton implementation
        static void create() { assert(!Engine::instance); Engine::instance = new Engine; }
        static void destroy() { assert(Engine::instance); delete Engine::instance; }
        static Engine *getInstance() { assert(Engine::instance); return Engine::instance; }
};
