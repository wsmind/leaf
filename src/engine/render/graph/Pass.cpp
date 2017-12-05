#include <engine/render/graph/Pass.h>

#include <engine/render/graph/GPUProfiler.h>

Pass::Pass(const std::string &name)
    : name(name)
    , depthStencilTarget(nullptr)
{
    // default viewport
    this->viewport.Width = 16.0f;
    this->viewport.Height = 16.0f;
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;
    this->viewport.TopLeftX = 0;
    this->viewport.TopLeftY = 0;
}

Pass::~Pass()
{
}

void Pass::setTargets(const std::vector<ID3D11RenderTargetView *> &colorTargets, ID3D11DepthStencilView *depthStencilTarget)
{
    this->colorTargets = colorTargets;
    this->depthStencilTarget = depthStencilTarget;
}

void Pass::setViewport(D3D11_VIEWPORT viewport)
{
    this->viewport = viewport;
}

void Pass::execute(ID3D11DeviceContext *context)
{
    GPUProfiler::ScopedProfile profile(this->name);

    context->RSSetViewports(1, &this->viewport);
    context->OMSetRenderTargets((UINT)this->colorTargets.size(), &this->colorTargets[0], this->depthStencilTarget);

    // render batches

    context->OMSetRenderTargets(0, nullptr, nullptr);
}
