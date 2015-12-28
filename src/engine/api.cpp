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

LEAFENGINE_API void leaf_render()
{
    Engine::getInstance()->render();
}

LEAFENGINE_API void leaf_render_blender_viewport()
{
    Engine::getInstance()->renderBlenderViewport();
}
