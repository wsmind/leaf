#include <engine/render/BloomRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>

#include <shaders/bloom.vs.hlsl.h>
#include <shaders/bloomthreshold.ps.hlsl.h>
#include <shaders/bloomdownsample.ps.hlsl.h>
#include <shaders/bloomaccumulation.ps.hlsl.h>

#include <shaders/gaussianblur.vs.hlsl.h>
#include <shaders/gaussianblurh.ps.hlsl.h>
#include <shaders/gaussianblurv.ps.hlsl.h>

BloomRenderer::BloomRenderer(int backbufferWidth, int backbufferHeight)
{
	this->backbufferWidth = backbufferWidth;
	this->backbufferHeight = backbufferHeight;

    HRESULT res;
	res = Device::device->CreateVertexShader(bloomVS, sizeof(bloomVS), NULL, &this->bloomVertexShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(bloomThresholdPS, sizeof(bloomThresholdPS), NULL, &this->bloomThresholdPixelShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(bloomDownsamplePS, sizeof(bloomDownsamplePS), NULL, &this->bloomDownsamplePixelShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(bloomAccumulationPS, sizeof(bloomAccumulationPS), NULL, &this->bloomAccumulationPixelShader); CHECK_HRESULT(res);

	res = Device::device->CreateVertexShader(gaussianBlurVS, sizeof(gaussianBlurVS), NULL, &this->gaussianBlurVertexShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(gaussianBlurHPS, sizeof(gaussianBlurHPS), NULL, &this->gaussianBlurHPixelShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(gaussianBlurVPS, sizeof(gaussianBlurVPS), NULL, &this->gaussianBlurVPixelShader); CHECK_HRESULT(res);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	res = Device::device->CreateInputLayout(layout, 4, bloomVS, sizeof(bloomVS), &this->inputLayout);
	CHECK_HRESULT(res);

	int width = backbufferWidth;
	int height = backbufferHeight;
	for (int i = 0; i < DOWNSAMPLE_LEVELS; i++)
	{
		width >>= 1;
		height >>= 1;

		this->downsampleTargets[i] = new RenderTarget(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
		this->blurTargets[i] = new RenderTarget(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}
}

BloomRenderer::~BloomRenderer()
{
	this->bloomVertexShader->Release();
	this->bloomThresholdPixelShader->Release();
	this->bloomDownsamplePixelShader->Release();
	this->bloomAccumulationPixelShader->Release();

	this->gaussianBlurVertexShader->Release();
	this->gaussianBlurHPixelShader->Release();
	this->gaussianBlurVPixelShader->Release();

	this->inputLayout->Release();

	for (int i = 0; i < DOWNSAMPLE_LEVELS; i++)
	{
		delete this->downsampleTargets[i];
		delete this->blurTargets[i];
	}
}

void BloomRenderer::render(FrameGraph *frameGraph, const RenderSettings &settings, RenderTarget *inputTarget, RenderTarget *outputTarget, Mesh *quad)
{
	Pass *thresholdPass = frameGraph->addPass("BloomThreshold");
	thresholdPass->setTargets({ this->downsampleTargets[0]->getTarget() }, nullptr);
	thresholdPass->setViewport((float)this->downsampleTargets[0]->getWidth(), (float)this->downsampleTargets[0]->getHeight(), glm::mat4(), glm::mat4());

	Batch *thresholdBatch = thresholdPass->addBatch("");
	thresholdBatch->setResources({ inputTarget->getSRV() });
	thresholdBatch->setSamplers({ inputTarget->getSamplerState() });
	thresholdBatch->setVertexShader(this->bloomVertexShader);
	thresholdBatch->setPixelShader(this->bloomThresholdPixelShader);
	thresholdBatch->setInputLayout(this->inputLayout);

	Job *thresholdJob = thresholdBatch->addJob();
	quad->setupJob(thresholdJob);
	thresholdJob->addInstance();

	// downsample
	for (int i = 1; i < DOWNSAMPLE_LEVELS; i++)
	{
		RenderTarget *source = this->downsampleTargets[i - 1];
		RenderTarget *destination = this->downsampleTargets[i];

		Pass *pass = frameGraph->addPass("BloomDownsample");
		pass->setTargets({ destination->getTarget() }, nullptr);
		pass->setViewport((float)destination->getWidth(), (float)destination->getHeight(), glm::mat4(), glm::mat4());

		Batch *batch = pass->addBatch("");
		batch->setResources({ source->getSRV() });
		batch->setSamplers({ source->getSamplerState() });
		batch->setVertexShader(this->bloomVertexShader);
		batch->setPixelShader(this->bloomDownsamplePixelShader);
		batch->setInputLayout(this->inputLayout);

		Job *job = batch->addJob();
		quad->setupJob(job);
		job->addInstance();
	}

	// upsample, blur and accumulate
	for (int i = DOWNSAMPLE_LEVELS - 2; i >= 0; i--)
	{
		RenderTarget *accumulator = (i == DOWNSAMPLE_LEVELS - 2) ? this->downsampleTargets[i + 1] : this->blurTargets[i + 1];
		RenderTarget *source = this->downsampleTargets[i];
		RenderTarget *destination = (i == 0) ? outputTarget : this->blurTargets[i];

		Pass *pass = frameGraph->addPass("BloomUpsample");
		pass->setTargets({ destination->getTarget() }, nullptr);
		pass->setViewport((float)destination->getWidth(), (float)destination->getHeight(), glm::mat4(), glm::mat4());

		Batch *batch = pass->addBatch("");
		batch->setResources({ source->getSRV(), accumulator->getSRV() });
		batch->setSamplers({ source->getSamplerState(), accumulator->getSamplerState() });
		batch->setVertexShader(this->bloomVertexShader);
		batch->setPixelShader(this->bloomAccumulationPixelShader);
		batch->setInputLayout(this->inputLayout);

		Job *job = batch->addJob();
		quad->setupJob(job);
		job->addInstance();
	}
}
