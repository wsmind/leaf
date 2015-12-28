#include <engine/api.h>
#include <engine/Engine.h>

LEAFENGINE_API void leaf_initialize()
{
    Engine::create();
}

LEAFENGINE_API void leaf_shutdown()
{
    Engine::destroy();
}

LEAFENGINE_API void leaf_render(int width, int height)
{
    Engine::getInstance()->render(width, height);
}

LEAFENGINE_API void leaf_render_blender_viewport(int width, int height)
{
    Engine::getInstance()->renderBlenderViewport(width, height);
}
