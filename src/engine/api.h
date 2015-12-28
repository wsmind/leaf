#pragma once

#ifdef LEAFENGINE_EXPORTS
#define LEAFENGINE_API extern "C" __declspec(dllexport)
#else
#define LEAFENGINE_API extern "C" __declspec(dllimport)
#endif

LEAFENGINE_API void leaf_initialize();
LEAFENGINE_API void leaf_shutdown();

LEAFENGINE_API void leaf_render();
LEAFENGINE_API void leaf_render_blender_viewport();
