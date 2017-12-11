#include <engine/render/MotionBlurRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>

#include <shaders/motionblur.vs.hlsl.h>
#include <shaders/motionblur.ps.hlsl.h>
#include <shaders/neighbormax.cs.hlsl.h>
#include <shaders/tilemax.cs.hlsl.h>

MotionBlurRenderer::MotionBlurRenderer(int backbufferWidth, int backbufferHeight, int tileSize)
{
    this->tileSize = tileSize;
    this->tileCountX = backbufferWidth / tileSize;
    this->tileCountY = backbufferHeight / tileSize;

    HRESULT res;
	res = Device::device->CreateVertexShader(motionblurVS, sizeof(motionblurVS), NULL, &this->motionblurVertexShader); CHECK_HRESULT(res);
	res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &this->motionblurPixelShader); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &this->tileMaxComputeShader); CHECK_HRESULT(res);
	res = Device::device->CreateComputeShader(neighborMaxCS, sizeof(neighborMaxCS), NULL, &this->neighborMaxComputeShader); CHECK_HRESULT(res);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	res = Device::device->CreateInputLayout(layout, 4, motionblurVS, sizeof(motionblurVS), &this->inputLayout);
	CHECK_HRESULT(res);

	D3D11_TEXTURE2D_DESC textureDesc;
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

    res = Device::device->CreateUnorderedAccessView(this->tileMaxTexture, NULL, &this->tileMaxUAV);
    CHECK_HRESULT(res);

	res = Device::device->CreateTexture2D(&textureDesc, NULL, &this->neighborMaxTexture);
	CHECK_HRESULT(res);

	res = Device::device->CreateShaderResourceView(this->neighborMaxTexture, NULL, &this->neighborMaxSRV);
	CHECK_HRESULT(res);

	res = Device::device->CreateUnorderedAccessView(this->neighborMaxTexture, NULL, &this->neighborMaxUAV);
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
	CHECK_HRESULT(res);
}

MotionBlurRenderer::~MotionBlurRenderer()
{
	this->motionblurVertexShader->Release();
	this->motionblurPixelShader->Release();
    
	this->tileMaxComputeShader->Release();
	this->neighborMaxComputeShader->Release();

	this->inputLayout->Release();

    this->tileMaxTexture->Release();
    this->tileMaxSRV->Release();
    this->tileMaxUAV->Release();

	this->neighborMaxTexture->Release();
	this->neighborMaxSRV->Release();
	this->neighborMaxUAV->Release();

	this->neighborMaxSampler->Release();
}

void MotionBlurRenderer::render(FrameGraph *frameGraph, RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget, int width, int height, Mesh *quad)
{
	Pass *tileMaxPass = frameGraph->addPass("TileMax");

	Batch *tileMaxBatch = tileMaxPass->addBatch("");
	tileMaxBatch->setResources({ motionTarget->getSRV() });
	tileMaxBatch->setUnorderedResources({ this->tileMaxUAV });
	tileMaxBatch->setComputeShader(this->tileMaxComputeShader);
	tileMaxBatch->addJob()->addDispatch(this->tileCountX, this->tileCountY, 1);

	Pass *neighborMaxPass = frameGraph->addPass("NeighborMax");

	Batch *neighborMaxBatch = neighborMaxPass->addBatch("");
	neighborMaxBatch->setResources({ this->tileMaxSRV });
	neighborMaxBatch->setUnorderedResources({ this->neighborMaxUAV });
	neighborMaxBatch->setComputeShader(this->neighborMaxComputeShader);
	neighborMaxBatch->addJob()->addDispatch(this->tileCountX, this->tileCountY, 1);

	Pass *blurPass = frameGraph->addPass("MotionBlur");
	blurPass->setTargets({ outputTarget->getTarget() }, nullptr);
	blurPass->setViewport((float)width, (float)height, glm::mat4(), glm::mat4());

	Batch *blurBatch = blurPass->addBatch("");
	blurBatch->setResources({ radianceTarget->getSRV(), motionTarget->getSRV(), this->neighborMaxSRV });
	blurBatch->setSamplers({ radianceTarget->getSamplerState(), motionTarget->getSamplerState(), this->neighborMaxSampler });
	blurBatch->setVertexShader(this->motionblurVertexShader);
	blurBatch->setPixelShader(this->motionblurPixelShader);
	blurBatch->setInputLayout(this->inputLayout);

	Job *blurJob = blurBatch->addJob();
	quad->setupJob(blurJob);
	blurJob->addInstance();
}
