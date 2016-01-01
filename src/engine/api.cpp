#include <engine/api.h>

#include <cassert>
#include <cstdio>

#include <engine/Engine.h>
#include <engine/cJSON/cJSON.h>

LEAFENGINE_API void leaf_initialize(int backbufferWidth, int backbufferHeight, bool capture)
{
    Engine::create();
    Engine::getInstance()->initialize(backbufferWidth, backbufferHeight, capture);
}

LEAFENGINE_API void leaf_shutdown()
{
    Engine::getInstance()->shutdown();
    Engine::destroy();
}

LEAFENGINE_API void leaf_load_data(const char *data)
{
    cJSON *json = cJSON_Parse(data);
    assert(json);
    Engine::getInstance()->loadData(json);
    cJSON_Delete(json);
}

LEAFENGINE_API void leaf_render(int width, int height)
{
    Engine::getInstance()->render(width, height);
}

LEAFENGINE_API void leaf_render_blender_viewport(int width, int height)
{
    Engine::getInstance()->renderBlenderViewport(width, height);
}
