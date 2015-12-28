#pragma once

#ifdef LEAFENGINE_EXPORTS
#define LEAFENGINE_API extern "C" __declspec(dllexport)
#else
#define LEAFENGINE_API extern "C" __declspec(dllimport)
#endif

LEAFENGINE_API void render_blender_viewport();
