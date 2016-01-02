#pragma once

#ifdef LEAFENGINE_EXPORTS
#define LEAFENGINE_API extern "C" __declspec(dllexport)
#else
#define LEAFENGINE_API extern "C" __declspec(dllimport)
#endif

LEAFENGINE_API void leaf_initialize(int backbufferWidth, int backbufferHeight, bool capture);
LEAFENGINE_API void leaf_shutdown();

LEAFENGINE_API void leaf_load_data(const char *data);

LEAFENGINE_API void leaf_render(int width, int height);
LEAFENGINE_API void leaf_render_blender_viewport(int width, int height, float view_matrix[], float projection_matrix[]);
