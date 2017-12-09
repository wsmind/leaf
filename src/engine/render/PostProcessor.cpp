#include <engine/render/PostProcessor.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Mesh.h>
#include <engine/render/MotionBlurRenderer.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/resource/ResourceManager.h>

#include <shaders/fxaa.vs.hlsl.h>
#include <shaders/fxaa.ps.hlsl.h>
#include <shaders/postprocess.vs.hlsl.h>
#include <shaders/postprocess.ps.hlsl.h>

PostProcessor::PostProcessor(ID3D11RenderTargetView *backbufferTarget, int backbufferWidth, int backbufferHeight)
	: backbufferTarget(backbufferTarget)
	, backbufferWidth(backbufferWidth)
	, backbufferHeight(backbufferHeight)
{
    HRESULT res;
    res = Device::device->CreateVertexShader(postprocessVS, sizeof(postprocessVS), NULL, &postprocessVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(postprocessPS, sizeof(postprocessPS), NULL, &postprocessPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(fxaaVS, sizeof(fxaaVS), NULL, &fxaaVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(fxaaPS, sizeof(fxaaPS), NULL, &fxaaPixelShader); CHECK_HRESULT(res);

    this->targets[0] = new RenderTarget(this->backbufferWidth, this->backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    this->targets[1] = new RenderTarget(this->backbufferWidth, this->backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");

    this->motionBlurRenderer = new MotionBlurRenderer(this->backbufferWidth, this->backbufferHeight, 40);
}

PostProcessor::~PostProcessor()
{
    this->postprocessVertexShader->Release();
    this->postprocessPixelShader->Release();
    this->fxaaVertexShader->Release();
    this->fxaaPixelShader->Release();

    delete this->targets[0];
    delete this->targets[1];

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    delete this->motionBlurRenderer;
}

void PostProcessor::render(FrameGraph *frameGraph, int width, int height, RenderTarget *motionTarget)
{
    this->motionBlurRenderer->render(frameGraph, this->targets[0], motionTarget, this->targets[1], this->backbufferWidth, this->backbufferHeight, this->fullscreenQuad);

    ID3D11RenderTargetView *target0 = this->targets[0]->getTarget();
    ID3D11RenderTargetView *target1 = this->targets[1]->getTarget();

    // tone mapping and gamma correction
    Pass *toneMappingPass = frameGraph->addPass("ToneMapping");
    toneMappingPass->setTargets({ this->targets[0]->getTarget() }, nullptr);

	D3D11_VIEWPORT viewport;
	viewport.Width = (float)this->backbufferWidth;
	viewport.Height = (float)this->backbufferHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	toneMappingPass->setViewport(viewport, glm::mat4(), glm::mat4());

    Batch *toneMappingBatch = toneMappingPass->addBatch("");
    toneMappingBatch->setResources({ this->targets[1]->getSRV() });
	toneMappingBatch->setSamplers({ this->targets[1]->getSamplerState() });
    toneMappingBatch->setVertexShader(postprocessVertexShader);
    toneMappingBatch->setPixelShader(postprocessPixelShader);

    Job *toneMappingJob = toneMappingBatch->addJob();
    this->fullscreenQuad->setupJob(toneMappingJob);
	toneMappingJob->addInstance();

    // fxaa pass and blit to backbuffer
    Pass *fxaaPass = frameGraph->addPass("FXAA");
    fxaaPass->setTargets({ this->backbufferTarget }, nullptr);

    D3D11_VIEWPORT outputViewport;
	outputViewport.Width = (float)width;
	outputViewport.Height = (float)height;
	outputViewport.MinDepth = 0.0f;
	outputViewport.MaxDepth = 1.0f;
	outputViewport.TopLeftX = 0;
	outputViewport.TopLeftY = 0;
    fxaaPass->setViewport(outputViewport, glm::mat4(), glm::mat4());

    Batch *fxaaBatch = fxaaPass->addBatch("");
    fxaaBatch->setResources({ this->targets[0]->getSRV() });
	fxaaBatch->setSamplers({ this->targets[0]->getSamplerState() });
	fxaaBatch->setVertexShader(fxaaVertexShader);
    fxaaBatch->setPixelShader(fxaaPixelShader);

    Job *fxaaJob = fxaaBatch->addJob();
    this->fullscreenQuad->setupJob(fxaaJob);
	fxaaJob->addInstance();
}
