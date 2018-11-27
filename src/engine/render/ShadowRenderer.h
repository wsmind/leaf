#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>

#include <engine/render/graph/Batch.h>

class FrameGraph;
class RenderList;
class Scene;
struct ShadowConstants;

struct DepthOnlyInstanceData
{
    glm::mat4 transformMatrix;
};

class ShadowRenderer
{
    public:
        ShadowRenderer(int resolution);
        ~ShadowRenderer();

        void render(FrameGraph *frameGraph, const Scene *scene, const RenderList *renderList, ID3D11InputLayout *inputLayout);

        DescriptorSet getParameterBlock() const { return this->parameterBlock; }

    private:
        int resolution;
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
        ID3D11DepthStencilState *depthState;

        DescriptorSet parameterBlock;
};
