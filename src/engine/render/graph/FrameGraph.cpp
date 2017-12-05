#include <engine/render/graph/FrameGraph.h>

#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Pass.h>

FrameGraph::FrameGraph(const std::string &profileFilename)
{
    GPUProfiler::create(!this->profileFilename.empty());
    GPUProfiler::getInstance()->beginJsonCapture();
}

FrameGraph::~FrameGraph()
{
    GPUProfiler::getInstance()->endJsonCapture(this->profileFilename);
    GPUProfiler::destroy();
}

void FrameGraph::addClearTarget(ID3D11RenderTargetView *target, glm::vec4 color)
{
    ClearColorTarget colorTarget;
    colorTarget.target = target;
    colorTarget.color = color;
    this->clearColorTargets.push_back(colorTarget);
}

void FrameGraph::addClearTarget(ID3D11DepthStencilView *target, float depth, unsigned char stencil)
{
    ClearDepthTarget depthTarget;
    depthTarget.target = target;
    depthTarget.depth = depth;
    depthTarget.stencil = stencil;
    this->clearDepthTargets.push_back(depthTarget);
}

Pass *FrameGraph::addPass(const std::string &name)
{
    Pass *pass = new Pass(name);
    this->passes.push_back(pass);
    return pass;
}

void FrameGraph::execute(ID3D11DeviceContext *context)
{
    GPUProfiler::getInstance()->beginFrame();

    this->clearAllTargets(context);
    this->executeAllPasses(context);

    GPUProfiler::getInstance()->endFrame();
}

void FrameGraph::clearAllTargets(ID3D11DeviceContext *context)
{
    GPUProfiler::ScopedProfile profile("Clear");

    for (auto &colorTarget : this->clearColorTargets)
        context->ClearRenderTargetView(colorTarget.target, (float *)&colorTarget.color);

    for (auto &depthTarget : this->clearDepthTargets)
        context->ClearDepthStencilView(depthTarget.target, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthTarget.depth, depthTarget.stencil);

    this->clearColorTargets.clear();
    this->clearDepthTargets.clear();
}

void FrameGraph::executeAllPasses(ID3D11DeviceContext *context)
{
    for (auto *pass : this->passes)
    {
        pass->execute(context);
        delete pass;
    }

    this->passes.clear();
}
