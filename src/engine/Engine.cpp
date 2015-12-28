#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <gl/GL.h>

Engine *Engine::instance = nullptr;

Engine::Engine()
{
    printf("LeafEngine started\n");
}

Engine::~Engine()
{
    printf("LeafEngine stopped\n");
}

void Engine::renderBlenderViewport()
{
    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
