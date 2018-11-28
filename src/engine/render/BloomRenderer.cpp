#include <engine/render/BloomRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/Mesh.h>
#include <engine/render/RenderSettings.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/Shaders.h>
#include <engine/render/ShaderCache.h>
#include <engine/render/ShaderVariant.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/render/shaders/constants/BloomConstants.h>

BloomRenderer::BloomRenderer(int backbufferWidth, int backbufferHeight)
{
	this->backbufferWidth = backbufferWidth;
	this->backbufferHeight = backbufferHeight;

	int width = backbufferWidth;
	int height = backbufferHeight;
	for (int i = 0; i < DOWNSAMPLE_LEVELS; i++)
	{
		width >>= 1;
		height >>= 1;

		this->downsampleTargets[i] = new RenderTarget(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
		this->blurTargets[i] = new RenderTarget(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}

    HRESULT res;

    D3D11_BUFFER_DESC cbDesc;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.ByteWidth = sizeof(BloomConstants);
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.StructureByteStride = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	res = Device::device->CreateBuffer(&cbDesc, nullptr, &this->constantBuffer);
	CHECK_HRESULT(res);
}

BloomRenderer::~BloomRenderer()
{
	for (int i = 0; i < DOWNSAMPLE_LEVELS; i++)
	{
		delete this->downsampleTargets[i];
		delete this->blurTargets[i];
	}

	this->constantBuffer->Release();
}

void BloomRenderer::render(FrameGraph *frameGraph, const RenderSettings &settings, RenderTarget *inputTarget, RenderTarget *outputTarget, const Mesh::SubMesh &quadSubMesh)
{
	BloomConstants bloomConstants;
	bloomConstants.threshold = settings.bloom.threshold;
	bloomConstants.intensity = settings.bloom.intensity;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT res = Device::context->Map(this->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CHECK_HRESULT(res);
	memcpy(mappedResource.pData, &bloomConstants, sizeof(bloomConstants));
	Device::context->Unmap(this->constantBuffer, 0);

	// early out with a simpler pass for debug
	if (settings.bloom.debug)
	{
		Pass *pass = frameGraph->addPass("BloomDebug");
		pass->setTargets({ outputTarget->getTarget() }, nullptr);
		pass->setViewport((float)outputTarget->getWidth(), (float)outputTarget->getHeight(), glm::mat4(1.0f), glm::mat4(1.0f));

        const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("bloomdebug");
        Pipeline pipeline = shaderVariant->getPipeline();
        pipeline.inputLayout = Shaders::layout.geometry2D;

        DescriptorSet bloomParameters = {
            { inputTarget->getSRV() },
            {},
            { inputTarget->getSamplerState() },
            { this->constantBuffer }
        };

        Batch *batch = pass->addBatch("");
        batch->setPipeline(pipeline);
        batch->setDescriptorSets({ bloomParameters });

		Job *job = batch->addJob();
        job->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
        job->addInstance();

		return;
	}

	Pass *thresholdPass = frameGraph->addPass("BloomThreshold");
	thresholdPass->setTargets({ this->downsampleTargets[0]->getTarget() }, nullptr);
	thresholdPass->setViewport((float)this->downsampleTargets[0]->getWidth(), (float)this->downsampleTargets[0]->getHeight(), glm::mat4(1.0f), glm::mat4(1.0f));

    const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("bloomthreshold");
    Pipeline pipeline = shaderVariant->getPipeline();
    pipeline.inputLayout = Shaders::layout.geometry2D;

    DescriptorSet bloomParameters = {
        { inputTarget->getSRV() },
        {},
        { inputTarget->getSamplerState() },
        { this->constantBuffer }
    };

	Batch *thresholdBatch = thresholdPass->addBatch("");
    thresholdBatch->setPipeline(pipeline);
    thresholdBatch->setDescriptorSets({ bloomParameters });

    Job *thresholdJob = thresholdBatch->addJob();
    thresholdJob->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
    thresholdJob->addInstance();

	// downsample
	for (int i = 1; i < (int)settings.bloom.size; i++)
	{
		RenderTarget *source = this->downsampleTargets[i - 1];
		RenderTarget *destination = this->downsampleTargets[i];

		Pass *pass = frameGraph->addPass("BloomDownsample");
		pass->setTargets({ destination->getTarget() }, nullptr);
		pass->setViewport((float)destination->getWidth(), (float)destination->getHeight(), glm::mat4(1.0f), glm::mat4(1.0f));

        const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("bloomdownsample");
        Pipeline pipeline = shaderVariant->getPipeline();
        pipeline.inputLayout = Shaders::layout.geometry2D;

        DescriptorSet bloomParameters = {
            { source->getSRV() },
            {},
            { source->getSamplerState() },
            { this->constantBuffer }
        };

        Batch *batch = pass->addBatch("");
        batch->setPipeline(pipeline);
        batch->setDescriptorSets({ bloomParameters });

		Job *job = batch->addJob();
        job->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
        job->addInstance();
	}

	// upsample, blur and accumulate
	for (int i = (int)settings.bloom.size - 2; i >= 0; i--)
	{
		RenderTarget *accumulator = (i == (int)settings.bloom.size - 2) ? this->downsampleTargets[i + 1] : this->blurTargets[i + 1];
		RenderTarget *source = (i == 0) ? inputTarget : this->downsampleTargets[i];
		RenderTarget *destination = (i == 0) ? outputTarget : this->blurTargets[i];

		Pass *pass = frameGraph->addPass("BloomUpsample");
		pass->setTargets({ destination->getTarget() }, nullptr);
		pass->setViewport((float)destination->getWidth(), (float)destination->getHeight(), glm::mat4(1.0f), glm::mat4(1.0f));

        const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("bloomaccumulation");
        Pipeline pipeline = shaderVariant->getPipeline();
        pipeline.inputLayout = Shaders::layout.geometry2D;

        DescriptorSet bloomParameters = {
            { source->getSRV(), accumulator->getSRV() },
            {},
            { source->getSamplerState(), accumulator->getSamplerState() },
            { this->constantBuffer }
        };

        Batch *batch = pass->addBatch("");
        batch->setPipeline(pipeline);
        batch->setDescriptorSets({ bloomParameters });

		Job *job = batch->addJob();
        job->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
        job->addInstance();
	}
}
