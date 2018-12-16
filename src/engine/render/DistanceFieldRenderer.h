#pragma once

#include <vector>

#include <engine/render/RenderList.h>

struct DescriptorSet;
class Pass;

class DistanceFieldRenderer
{
    public:
        DistanceFieldRenderer();
        ~DistanceFieldRenderer();

        void setRenderList(const RenderList *renderList);
        void clearRenderList();

        void addPrePassJobs(Pass *pass);
        void addDeferredJobs(Pass *pass, const DescriptorSet &shadowParameterBlock, const DescriptorSet &environmentParameterBlock);

    private:
        std::vector<RenderList::DistanceField> distanceFields;

        ID3D11DepthStencilState *raymarchDepthStencilState;
        ID3D11DepthStencilState *deferredDepthStencilState;
};
