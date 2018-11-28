#include <engine/render/ShadowRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/RenderList.h>
#include <engine/render/Shaders.h>
#include <engine/render/ShaderCache.h>
#include <engine/render/ShaderVariant.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/scene/Scene.h>

#include <engine/render/shaders/constants/ShadowConstants.h>

ShadowRenderer::ShadowRenderer(int resolution)
{
    HRESULT res;

    this->resolution = resolution;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Width = resolution * 2;
    shadowMapDesc.Height = resolution * 2;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    res = Device::device->CreateTexture2D(&shadowMapDesc, NULL, &this->shadowMap);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC targetDesc;
    ZeroMemory(&targetDesc, sizeof(targetDesc));
    targetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    targetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(this->shadowMap, &targetDesc, &this->target);
    CHECK_HRESULT(res);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    ID3D11ShaderResourceView *srv;
    res = Device::device->CreateShaderResourceView(this->shadowMap, &srvDesc, &srv);
    CHECK_HRESULT(res);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    ID3D11SamplerState *sampler;
    res = Device::device->CreateSamplerState(&samplerDesc, &sampler);
    CHECK_HRESULT(res);
    D3D11_DEPTH_STENCIL_DESC depthStateDesc;

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));

    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    res = Device::device->CreateDepthStencilState(&depthStateDesc, &this->depthState);
    CHECK_HRESULT(res);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(ShadowConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer *constantBuffer;
    res = Device::device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
    CHECK_HRESULT(res);

    this->parameterBlock.resources = { srv };
    this->parameterBlock.samplers = { sampler };
    this->parameterBlock.constants = { constantBuffer };
}

ShadowRenderer::~ShadowRenderer()
{
    this->shadowMap->Release();
    this->target->Release();
    this->depthState->Release();

    this->parameterBlock.resources[0]->Release();
    this->parameterBlock.samplers[0]->Release();
    this->parameterBlock.constants[0]->Release();
}

void ShadowRenderer::render(FrameGraph *frameGraph, const Scene *scene, const RenderList *renderList)
{
    const std::vector<RenderList::Job> &jobs = renderList->getJobs();
    const std::vector<RenderList::Light> &lights = renderList->getLights();

	frameGraph->addClearTarget(this->target, 1.0f, 0);

    ShadowConstants shadowConstants;
    ZeroMemory(&shadowConstants, sizeof(shadowConstants));

    int shadowCount = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		// only spotlights cast shadows
		if (!lights[i].spot || shadowCount >= 4)
			continue;

		int index = shadowCount++;

		// apply NDC [-1, 1] to texture space [0, 1] to atlas rect
		/*glm::vec3 scale = glm::vec3(0.5f, 0.5f, 1.0f);
		glm::vec3 offset = glm::vec3(0.5f, 0.5f, 0.0f);
		//glm::vec3 scale = glm::vec3(0.125f, 0.125f, 1.0f);
		//glm::vec3 offset = glm::vec3(0.125f + 0.25f * (float)(index % 4), 0.125f + 0.25f * (float)(index / 4), 0.0f);
		glm::mat4 biasMatrix(
			scale.x, 0.0f, 0.0f, offset.x,
			0.0f, scale.y, 0.0f, offset.y,
			0.0f, 0.0f, scale.z, offset.z,
			0.0f, 0.0f, 0.0f, 1.0f
		);*/
		shadowConstants.lightMatrix[index] = lights[i].shadowTransform;

		GPUProfiler::ScopedProfile profile("Shadow");

		Pass *shadowPass = frameGraph->addPass("ShadowMap");
		shadowPass->setTargets({}, this->target);

		D3D11_VIEWPORT viewport;
        viewport.Width = (float)this->resolution;
        viewport.Height = (float)this->resolution;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = (float)((index % 2) * this->resolution);
        viewport.TopLeftY = (float)((index / 2) * this->resolution);
        shadowPass->setViewport(viewport, glm::mat4(1.0f), glm::mat4(1.0f));

        const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("depthonly");
        Pipeline pipeline = shaderVariant->getPipeline();
        pipeline.inputLayout = Shaders::layout.depthOnly;
        pipeline.depthStencil = this->depthState;

        Batch *batch = shadowPass->addBatch("");
        batch->setPipeline(pipeline);

        const Mesh::SubMesh *currentSubMesh = nullptr;
		Job *currentJob = nullptr;
        for (const auto &job : jobs)
        {
            if (currentSubMesh != job.subMesh)
            {
                currentSubMesh = job.subMesh;

				currentJob = batch->addJob();
                currentJob->setBuffers(currentSubMesh->vertexBuffer, currentSubMesh->indexBuffer, currentSubMesh->indexCount);
            }

			DepthOnlyInstanceData instanceData;
			instanceData.transformMatrix = lights[i].shadowTransform * job.transform;
	
			currentJob->addInstance(instanceData);
		}
    }

    // update shader constants
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->parameterBlock.constants[0], 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &shadowConstants, sizeof(shadowConstants));
    Device::context->Unmap(this->parameterBlock.constants[0], 0);
}
