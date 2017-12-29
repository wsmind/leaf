#include <engine/render/BloomRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>

#include <shaders/bloom.vs.hlsl.h>
#include <shaders/bloomThreshold.ps.hlsl.h>

BloomRenderer::BloomRenderer(int backbufferWidth, int backbufferHeight)
{
	this->backbufferWidth = backbufferWidth;
	this->backbufferHeight = backbufferHeight;

    HRESULT res;
	res = Device::device->CreateVertexShader(bloomVS, sizeof(bloomVS), NULL, &this->bloomVertexShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(bloomThresholdPS, sizeof(bloomThresholdPS), NULL, &this->bloomThresholdPixelShader); CHECK_HRESULT(res);

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
	}

	/*D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = this->tileCountX;
    textureDesc.Height = this->tileCountY;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    res = Device::device->CreateTexture2D(&textureDesc, NULL, &this->tileMaxTexture);
    CHECK_HRESULT(res);

    res = Device::device->CreateShaderResourceView(this->tileMaxTexture, NULL, &this->tileMaxSRV);
    CHECK_HRESULT(res);

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	res = Device::device->CreateSamplerState(&samplerDesc, &this->neighborMaxSampler);
	CHECK_HRESULT(res);*/
}

BloomRenderer::~BloomRenderer()
{
	this->bloomVertexShader->Release();
	this->bloomThresholdPixelShader->Release();

	this->inputLayout->Release();

	for (int i = 0; i < DOWNSAMPLE_LEVELS; i++)
	{
		delete this->downsampleTargets[i];
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

	// downsample and blur
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
		batch->setPixelShader(this->bloomThresholdPixelShader);
		batch->setInputLayout(this->inputLayout);

		Job *job = batch->addJob();
		quad->setupJob(job);
		job->addInstance();
	}

	// accumulate all blur levels into result
	Pass *accumulationPass = frameGraph->addPass("BloomAccumulation");
	accumulationPass->setTargets({ outputTarget->getTarget() }, nullptr);
	accumulationPass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, glm::mat4(), glm::mat4());

	Batch *accumulationBatch = thresholdPass->addBatch("");
	accumulationBatch->setResources({ inputTarget->getSRV(), this->downsampleTargets[0]->getSRV(), this->downsampleTargets[1]->getSRV(), this->downsampleTargets[2]->getSRV(), this->downsampleTargets[3]->getSRV() });
	accumulationBatch->setSamplers({ inputTarget->getSamplerState(), this->downsampleTargets[0]->getSamplerState(), this->downsampleTargets[1]->getSamplerState(), this->downsampleTargets[2]->getSamplerState(), this->downsampleTargets[3]->getSamplerState() });
	accumulationBatch->setVertexShader(this->bloomVertexShader);
	accumulationBatch->setPixelShader(this->bloomThresholdPixelShader);
	accumulationBatch->setInputLayout(this->inputLayout);

	Job *accumulationJob = accumulationBatch->addJob();
	quad->setupJob(accumulationJob);
	accumulationJob->addInstance();
}
