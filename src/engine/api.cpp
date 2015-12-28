#include <engine/api.h>
#include <engine/Renderer.h>

LEAFENGINE_API void plop()
{
    Renderer *r = new Renderer;
    r->render(nullptr);
    delete r;
}
