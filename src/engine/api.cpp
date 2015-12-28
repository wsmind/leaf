#include <engine/api.h>
#include <engine/Engine.h>

LEAFENGINE_API void render_blender_viewport()
{
    Engine::getInstance()->renderBlenderViewport();
}
