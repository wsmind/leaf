#pragma once

#include <cassert>

class Engine
{
    public:
        Engine();
        ~Engine();

        void renderBlenderViewport();

    private:
        static Engine *instance;

    public:
        // singleton implementation
        static void create() { assert(!Engine::instance); Engine::instance = new Engine; }
        static void destroy() { assert(Engine::instance); delete Engine::instance; }
        static Engine *getInstance() { assert(Engine::instance); return Engine::instance; }
};
