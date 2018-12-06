#pragma once

#ifdef LEAFENGINE_EXPORTS
#define LEAFENGINE_API extern "C" __declspec(dllexport)
#else
#define LEAFENGINE_API extern "C" __declspec(dllimport)
#endif

LEAFENGINE_API void leaf_initialize(int backbufferWidth, int backbufferHeight, bool capture, const char *profileFilename, const char *shaderPath);
LEAFENGINE_API void leaf_shutdown();

LEAFENGINE_API void leaf_load_data(const void *buffer, size_t size);
LEAFENGINE_API int leaf_export_data(const void *buffer, size_t size, const char *exportPath);

LEAFENGINE_API void leaf_update(float time);

LEAFENGINE_API void leaf_render(int width, int height, float deltaTime);
LEAFENGINE_API void leaf_render_blender_viewport(int width, int height, float view_matrix[], float projection_matrix[]);
LEAFENGINE_API void leaf_render_blender_frame(const char *sceneName, void *pass, float time);
