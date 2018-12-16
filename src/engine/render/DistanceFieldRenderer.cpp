#include <engine/render/DistanceFieldRenderer.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <engine/render/Material.h>
#include <engine/render/Shaders.h>
#include <engine/render/ShaderCache.h>
#include <engine/render/ShaderVariant.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>

#pragma pack(push)
#pragma pack(16)
struct DistanceFieldInstanceData
{
    glm::mat4 modelMatrix;
    glm::mat4 modelMatrixInverse;
    glm::mat3x4 normalMatrix; // use 3x4 to match cbuffer packing rules
    glm::mat3x4 normalMatrixInverse; // use 3x4 to match cbuffer packing rules
};
#pragma pack(pop)

DistanceFieldRenderer::DistanceFieldRenderer()
{
    D3D11_DEPTH_STENCIL_DESC depthStateDesc;

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));
    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStateDesc.StencilEnable = TRUE;
    depthStateDesc.StencilReadMask = 0x00;
    depthStateDesc.StencilWriteMask = 0xff;
    depthStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    depthStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->raymarchDepthStencilState);

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));
    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStateDesc.StencilEnable = TRUE;
    depthStateDesc.StencilReadMask = 0xff;
    depthStateDesc.StencilWriteMask = 0x00;
    depthStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    depthStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->deferredDepthStencilState);
}

DistanceFieldRenderer::~DistanceFieldRenderer()
{
    this->raymarchDepthStencilState->Release();
    this->deferredDepthStencilState->Release();
}

void DistanceFieldRenderer::setRenderList(const RenderList *renderList)
{
    this->distanceFields = renderList->getDistanceFields();
}

void DistanceFieldRenderer::clearRenderList()
{
    this->distanceFields.clear();
}

void DistanceFieldRenderer::addPrePassJobs(Pass *pass)
{
    ShaderCache::Hash currentHash = { 0, 0 };
    Batch *currentBatch = nullptr;
    const Mesh::SubMesh *currentSubMesh = nullptr;
    Job *currentJob = nullptr;
    unsigned int index = 0;
    for (const auto &sdf : this->distanceFields)
    {
        index++;

        if (sdf.prefixHash == ShaderCache::Hash({ 0, 0 }))
            continue;

        if (sdf.prefixHash != currentHash)
        {
            currentHash = sdf.prefixHash;

            const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("sdf-gbuffer", currentHash);
            Pipeline pipeline = shaderVariant->getPipeline();
            pipeline.inputLayout = Shaders::layout.distanceField;
            pipeline.depthStencil = this->raymarchDepthStencilState;
            pipeline.stencilRef = index;

            currentBatch = pass->addBatch("SDF");
            currentBatch->setPipeline(pipeline);
        }

        if (currentSubMesh != sdf.subMesh)
        {
            currentSubMesh = sdf.subMesh;

            currentJob = currentBatch->addJob();
            currentJob->setBuffers(currentSubMesh->vertexBuffer, currentSubMesh->indexBuffer, currentSubMesh->indexCount);
        }

        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(sdf.transform));

        DistanceFieldInstanceData instanceData;
        instanceData.modelMatrix = sdf.transform;
        instanceData.modelMatrixInverse = glm::inverse(sdf.transform);
        instanceData.normalMatrix = glm::mat3x4(normalMatrix);
        instanceData.normalMatrixInverse = glm::mat3x4(glm::inverse(normalMatrix));

        currentJob->addInstance(instanceData);
    }
}

void DistanceFieldRenderer::addDeferredJobs(Pass *pass, const DescriptorSet &sdfGbufferParameterBlock, const DescriptorSet &shadowParameterBlock, const DescriptorSet &environmentParameterBlock)
{
    ShaderCache::Hash currentHash = { 0, 0 };
    Batch *currentBatch = nullptr;
    const Mesh::SubMesh *currentSubMesh = nullptr;
    Job *currentJob = nullptr;
    unsigned int index = 0;
    for (const auto &sdf : this->distanceFields)
    {
        index++;

        if (sdf.material->getPrefixHash() != currentHash)
        {
            currentHash = sdf.material->getPrefixHash();

            const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("sdf-deferred", currentHash);
            Pipeline pipeline = shaderVariant->getPipeline();
            pipeline.inputLayout = Shaders::layout.distanceField;
            pipeline.depthStencil = this->deferredDepthStencilState;
            pipeline.stencilRef = index;

            currentBatch = pass->addBatch("SDF");
            currentBatch->setPipeline(pipeline);
            currentBatch->setDescriptorSets({
                sdf.material->getParameterBlock(),
                sdfGbufferParameterBlock,
                shadowParameterBlock,
                environmentParameterBlock
            });
        }

        if (currentSubMesh != sdf.subMesh)
        {
            currentSubMesh = sdf.subMesh;

            currentJob = currentBatch->addJob();
            currentJob->setBuffers(currentSubMesh->vertexBuffer, currentSubMesh->indexBuffer, currentSubMesh->indexCount);
        }

        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(sdf.transform));

        DistanceFieldInstanceData instanceData;
        instanceData.modelMatrix = sdf.transform;
        instanceData.modelMatrixInverse = glm::inverse(sdf.transform);
        instanceData.normalMatrix = glm::mat3x4(normalMatrix);
        instanceData.normalMatrixInverse = glm::mat3x4(glm::inverse(normalMatrix));

        currentJob->addInstance(instanceData);
    }
}
