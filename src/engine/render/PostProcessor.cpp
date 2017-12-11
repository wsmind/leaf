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

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	res = Device::device->CreateInputLayout(layout, 4, postprocessVS, sizeof(postprocessVS), &this->inputLayout);
	CHECK_HRESULT(res);

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

	this->inputLayout->Release();

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
	toneMappingPass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, glm::mat4(), glm::mat4());

    Batch *toneMappingBatch = toneMappingPass->addBatch("");
    toneMappingBatch->setResources({ this->targets[1]->getSRV() });
	toneMappingBatch->setSamplers({ this->targets[1]->getSamplerState() });
    toneMappingBatch->setVertexShader(postprocessVertexShader);
    toneMappingBatch->setPixelShader(postprocessPixelShader);
	toneMappingBatch->setInputLayout(this->inputLayout);

    Job *toneMappingJob = toneMappingBatch->addJob();
    this->fullscreenQuad->setupJob(toneMappingJob);
	toneMappingJob->addInstance();

    // fxaa pass and blit to backbuffer
    Pass *fxaaPass = frameGraph->addPass("FXAA");
    fxaaPass->setTargets({ this->backbufferTarget }, nullptr);
	fxaaPass->setViewport((float)width, (float)height, glm::mat4(), glm::mat4());

    Batch *fxaaBatch = fxaaPass->addBatch("");
    fxaaBatch->setResources({ this->targets[0]->getSRV() });
	fxaaBatch->setSamplers({ this->targets[0]->getSamplerState() });
	fxaaBatch->setVertexShader(fxaaVertexShader);
    fxaaBatch->setPixelShader(fxaaPixelShader);
	fxaaBatch->setInputLayout(this->inputLayout);

    Job *fxaaJob = fxaaBatch->addJob();
    this->fullscreenQuad->setupJob(fxaaJob);
	fxaaJob->addInstance();
}
