#include <engine/render/graph/FrameGraph.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/render/shaders/constants/SceneConstants.h>
#include <engine/render/shaders/constants/PassConstants.h>

FrameGraph::FrameGraph(const std::string &profileFilename)
{
    HRESULT res;

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(SceneConstants);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->sceneConstantBuffer);
    CHECK_HRESULT(res);

    cbDesc.ByteWidth = sizeof(PassConstants);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->passConstantBuffer);
    CHECK_HRESULT(res);

    Device::device->GetImmediateContext(&this->context);

	res = this->context->QueryInterface(__uuidof(this->annotation), (void **)&this->annotation);
	CHECK_HRESULT(res);

	Job::createInstanceBuffer(1 * 1024 * 1024);

    this->profileFilename = profileFilename;

    GPUProfiler::create(!this->profileFilename.empty(), this->context);
    GPUProfiler::getInstance()->beginJsonCapture();
}

FrameGraph::~FrameGraph()
{
    GPUProfiler::getInstance()->endJsonCapture(this->profileFilename);
    GPUProfiler::destroy();

	Job::destroyInstanceBuffer();

	this->context->Release();
	this->annotation->Release();

    this->sceneConstantBuffer->Release();
    this->passConstantBuffer->Release();
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

void FrameGraph::execute(const SceneConstants &sceneConstants)
{
    GPUProfiler::getInstance()->beginFrame();

	Job::applyInstanceBuffer();

    // upload scene constants to GPU
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = this->context->Map(this->sceneConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &sceneConstants, sizeof(SceneConstants));
    this->context->Unmap(this->sceneConstantBuffer, 0);

    // bind common buffers
    ID3D11Buffer *commonConstantBuffers[] = { this->sceneConstantBuffer, this->passConstantBuffer };
    Device::context->VSSetConstantBuffers(0, 2, commonConstantBuffers);
    Device::context->PSSetConstantBuffers(0, 2, commonConstantBuffers);

    this->clearAllTargets();
    this->executeAllPasses();

    // unbind common buffers
    ID3D11Buffer *nullConstantBuffers[] = { nullptr, nullptr };
    Device::context->VSSetConstantBuffers(0, 2, nullConstantBuffers);
    Device::context->PSSetConstantBuffers(0, 2, nullConstantBuffers);

	Job::resetInstanceBufferPosition();

	GPUProfiler::getInstance()->endFrame();
}

void FrameGraph::clearAllTargets()
{
    GPUProfiler::ScopedProfile profile("Clear");

	std::string clearName("Clear");
	std::wstring clearNameWide(clearName.begin(), clearName.end());
	this->annotation->BeginEvent(clearNameWide.c_str());

	for (auto &colorTarget : this->clearColorTargets)
        this->context->ClearRenderTargetView(colorTarget.target, (float *)&colorTarget.color);

    for (auto &depthTarget : this->clearDepthTargets)
        this->context->ClearDepthStencilView(depthTarget.target, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthTarget.depth, depthTarget.stencil);

    this->clearColorTargets.clear();
    this->clearDepthTargets.clear();

	this->annotation->EndEvent();
}

void FrameGraph::executeAllPasses()
{
    for (auto *pass : this->passes)
    {
        pass->execute(this->context, this->passConstantBuffer, this->annotation);
        delete pass;
    }

    this->passes.clear();
}
