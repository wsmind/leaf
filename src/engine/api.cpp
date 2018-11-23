#include <engine/api.h>

#include <cassert>
#include <cstdio>

#include <engine/Engine.h>
#include <cJSON/cJSON.h>

LEAFENGINE_API void leaf_initialize(int backbufferWidth, int backbufferHeight, bool capture, const char *profileFilename, const char *shaderPath)
{
    Engine::create();
    Engine::getInstance()->initialize(backbufferWidth, backbufferHeight, capture, profileFilename != nullptr ? profileFilename : "", shaderPath != nullptr ? shaderPath : "");
}

LEAFENGINE_API void leaf_shutdown()
{
    Engine::getInstance()->shutdown();
    Engine::destroy();
}

LEAFENGINE_API void leaf_load_data(const void *buffer, size_t size)
{
    assert(buffer);
    Engine::getInstance()->loadData(buffer, size);
}

LEAFENGINE_API void leaf_update(float time)
{
    Engine::getInstance()->update(time);
}

LEAFENGINE_API void leaf_render(int width, int height, float deltaTime)
{
    Engine::getInstance()->render(width, height, deltaTime);
}

LEAFENGINE_API void leaf_render_blender_viewport(int width, int height, float view_matrix[], float projection_matrix[])
{
    glm::mat4 viewMatrix(
        view_matrix[0], view_matrix[1], view_matrix[2], view_matrix[3],
        view_matrix[4], view_matrix[5], view_matrix[6], view_matrix[7],
        view_matrix[8], view_matrix[9], view_matrix[10], view_matrix[11],
        view_matrix[12], view_matrix[13], view_matrix[14], view_matrix[15]
        );
    glm::mat4 projectionMatrix(
        projection_matrix[0], projection_matrix[1], projection_matrix[2], projection_matrix[3],
        projection_matrix[4], projection_matrix[5], projection_matrix[6], projection_matrix[7],
        projection_matrix[8], projection_matrix[9], projection_matrix[10], projection_matrix[11],
        projection_matrix[12], projection_matrix[13], projection_matrix[14], projection_matrix[15]
        );
    Engine::getInstance()->renderBlenderViewport(width, height, viewMatrix, projectionMatrix);
}

// memory layout taken from RE_pipeline.h
struct RenderPass {
    struct RenderPass *next, *prev;
    int channels;
    char name[64];		/* amount defined in openexr_multi.h */
    char chan_id[8];	/* amount defined in openexr_multi.h */
    float *rect;
    int rectx, recty;

    char fullname[64]; /* EXR_PASS_MAXNAME */
    char view[64];		/* EXR_VIEW_MAXNAME */
    int view_id;	/* quick lookup */

    int pad;
};

LEAFENGINE_API void leaf_render_blender_frame(const char *sceneName, void *pass, float time)
{
    // hack; get the pass pointer directly from the blender object,
    // to be able to copy image data directly
    RenderPass *renderPass = (RenderPass *)pass;

    // only RGBA renders supported right now
    assert(renderPass->channels == 4);

    Engine::getInstance()->renderBlenderFrame(sceneName, renderPass->rectx, renderPass->recty, renderPass->rect, time);
}
