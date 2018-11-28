#include <engine/render/MotionBlurRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/Shaders.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>

#include <shaders/motionblur.vs.hlsl.h>

MotionBlurRenderer::MotionBlurRenderer(int backbufferWidth, int backbufferHeight, int tileSize)
{
    this->tileSize = tileSize;
    this->tileCountX = backbufferWidth / tileSize;
    this->tileCountY = backbufferHeight / tileSize;

    HRESULT res;

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
    this->tileMaxTexture->Release();
    this->tileMaxSRV->Release();
    this->tileMaxUAV->Release();

	this->neighborMaxTexture->Release();
	this->neighborMaxSRV->Release();
	this->neighborMaxUAV->Release();

	this->neighborMaxSampler->Release();
}

void MotionBlurRenderer::render(FrameGraph *frameGraph, RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget, int width, int height, const Mesh::SubMesh &quadSubMesh)
{
	Pass *tileMaxPass = frameGraph->addPass("TileMax");

	Batch *tileMaxBatch = tileMaxPass->addBatch("");
	tileMaxBatch->setResources({ motionTarget->getSRV() });
	tileMaxBatch->setUnorderedResources({ this->tileMaxUAV });
	tileMaxBatch->setComputeShader(Shaders::compute.tileMax);
	tileMaxBatch->addJob()->addDispatch(this->tileCountX, this->tileCountY, 1);

	Pass *neighborMaxPass = frameGraph->addPass("NeighborMax");

	Batch *neighborMaxBatch = neighborMaxPass->addBatch("");
	neighborMaxBatch->setResources({ this->tileMaxSRV });
	neighborMaxBatch->setUnorderedResources({ this->neighborMaxUAV });
	neighborMaxBatch->setComputeShader(Shaders::compute.neighborMax);
	neighborMaxBatch->addJob()->addDispatch(this->tileCountX, this->tileCountY, 1);

	Pass *blurPass = frameGraph->addPass("MotionBlur");
	blurPass->setTargets({ outputTarget->getTarget() }, nullptr);
	blurPass->setViewport((float)width, (float)height, glm::mat4(1.0f), glm::mat4(1.0f));

	Batch *blurBatch = blurPass->addBatch("");
	blurBatch->setResources({ radianceTarget->getSRV(), motionTarget->getSRV(), this->neighborMaxSRV });
	blurBatch->setSamplers({ radianceTarget->getSamplerState(), motionTarget->getSamplerState(), this->neighborMaxSampler });
	blurBatch->setVertexShader(Shaders::vertex.motionBlur);
	blurBatch->setPixelShader(Shaders::pixel.motionBlur);
	blurBatch->setInputLayout(Shaders::layout.geometry2D);

	Job *blurJob = blurBatch->addJob();
    blurJob->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
    blurJob->addInstance();
}
