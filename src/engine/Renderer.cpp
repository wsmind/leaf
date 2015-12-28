#include <engine/Renderer.h>

#include <cstdio>

#include <windows.h>
#include <gl/GL.h>

void Renderer::render(const Scene *scene)
{
    printf("rendering!\n");

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
