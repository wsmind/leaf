#include <engine/render/PostProcessor.h>

#include <engine/render/BloomRenderer.h>
#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Mesh.h>
#include <engine/render/MotionBlurRenderer.h>
#include <engine/render/RenderSettings.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/Shaders.h>
#include <engine/render/ShaderCache.h>
#include <engine/render/ShaderVariant.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/render/shaders/constants/PostProcessConstants.h>
#include <engine/resource/ResourceManager.h>

#include <shaders/postprocess.vs.hlsl.h>

PostProcessor::PostProcessor(ID3D11RenderTargetView *backbufferTarget, int backbufferWidth, int backbufferHeight)
	: backbufferTarget(backbufferTarget)
	, backbufferWidth(backbufferWidth)
	, backbufferHeight(backbufferHeight)
{
    HRESULT res;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	res = Device::device->CreateInputLayout(layout, 4, postprocessVS, sizeof(postprocessVS), &this->inputLayout);
	CHECK_HRESULT(res);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(PostProcessConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    res = Device::device->CreateBuffer(&cbDesc, nullptr, &this->constantBuffer);
    CHECK_HRESULT(res);

    this->targets[0] = new RenderTarget(this->backbufferWidth, this->backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    this->targets[1] = new RenderTarget(this->backbufferWidth, this->backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");

    this->motionBlurRenderer = new MotionBlurRenderer(this->backbufferWidth, this->backbufferHeight, 40);
	this->bloomRenderer = new BloomRenderer(this->backbufferWidth, this->backbufferHeight);
}

PostProcessor::~PostProcessor()
{
	this->inputLayout->Release();

    this->constantBuffer->Release();

    delete this->targets[0];
    delete this->targets[1];

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    delete this->motionBlurRenderer;
	delete this->bloomRenderer;
}

void PostProcessor::render(FrameGraph *frameGraph, const RenderSettings &settings, RenderTarget *motionTarget)
{
    PostProcessConstants postProcessConstants;
    postProcessConstants.pixellateDivider = settings.postProcess.pixellateDivider;
    postProcessConstants.vignetteSize = settings.postProcess.vignetteSize;
    postProcessConstants.vignettePower = settings.postProcess.vignettePower;
    postProcessConstants.abberationStrength = settings.postProcess.abberationStrength;
    postProcessConstants.scanlineStrength = settings.postProcess.scanlineStrength;
    postProcessConstants.scanlineFrequency = settings.postProcess.scanlineFrequency;
    postProcessConstants.scanlineOffset = settings.postProcess.scanlineOffset;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &postProcessConstants, sizeof(postProcessConstants));
    Device::context->Unmap(this->constantBuffer, 0);

    const Mesh::SubMesh &quadSubMesh = this->fullscreenQuad->getSubMeshes()[0];

    this->motionBlurRenderer->render(frameGraph, this->targets[0], motionTarget, this->targets[1], this->backbufferWidth, this->backbufferHeight, quadSubMesh);

	this->bloomRenderer->render(frameGraph, settings, this->targets[1], this->targets[0], quadSubMesh);

    // tone mapping and gamma correction
    Pass *toneMappingPass = frameGraph->addPass("ToneMapping");
    toneMappingPass->setTargets({ this->targets[1]->getTarget() }, nullptr);
	toneMappingPass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, glm::mat4(1.0f), glm::mat4(1.0f));

    const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("postprocess");
    Pipeline pipeline = shaderVariant->getPipeline();
    pipeline.inputLayout = this->inputLayout;

    Batch *toneMappingBatch = toneMappingPass->addBatch("");
    toneMappingBatch->setPipeline(pipeline);
    toneMappingBatch->setDescriptorSets({ {
        { this->targets[0]->getSRV() },
        {},
        { this->targets[0]->getSamplerState() },
        { this->constantBuffer }
    } });

    Job *toneMappingJob = toneMappingBatch->addJob();
    toneMappingJob->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
	toneMappingJob->addInstance();

    // fxaa pass and blit to backbuffer
    Pass *fxaaPass = frameGraph->addPass("FXAA");
    fxaaPass->setTargets({ this->backbufferTarget }, nullptr);
	fxaaPass->setViewport((float)settings.frameWidth, (float)settings.frameHeight, glm::mat4(1.0f), glm::mat4(1.0f));

    Batch *fxaaBatch = fxaaPass->addBatch("");
    fxaaBatch->setResources({ this->targets[1]->getSRV() });
	fxaaBatch->setSamplers({ this->targets[1]->getSamplerState() });
	fxaaBatch->setVertexShader(Shaders::vertex.fxaa);
    fxaaBatch->setPixelShader(Shaders::pixel.fxaa);
	fxaaBatch->setInputLayout(this->inputLayout);

    Job *fxaaJob = fxaaBatch->addJob();
    fxaaJob->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
    fxaaJob->addInstance();
}
