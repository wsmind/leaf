#include <engine/render/graph/Batch.h>

#include <cassert>

#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Job.h>

namespace
{
    // base implementation left intentionally undefined; will break the build if an unknown
    // shader type is encountered
    template <class StageType>
    void bindStage(ID3D11DeviceContext *context, StageType *shader, const DescriptorSet &descriptors);

    template <>
    void bindStage<ID3D11VertexShader>(ID3D11DeviceContext *context, ID3D11VertexShader *shader, const DescriptorSet &descriptors)
    {
        context->VSSetShader(shader, nullptr, 0);
        context->VSSetConstantBuffers(2, (UINT)descriptors.constants.size(), descriptors.constants.data());
        context->VSSetShaderResources(0, (UINT)descriptors.resources.size(), descriptors.resources.data());
        context->VSSetSamplers(0, (UINT)descriptors.samplers.size(), descriptors.samplers.data());
    }

    template <>
    void bindStage<ID3D11PixelShader>(ID3D11DeviceContext *context, ID3D11PixelShader *shader, const DescriptorSet &descriptors)
    {
        context->PSSetShader(shader, nullptr, 0);
        context->PSSetConstantBuffers(2, (UINT)descriptors.constants.size(), descriptors.constants.data());
        context->PSSetShaderResources(0, (UINT)descriptors.resources.size(), descriptors.resources.data());
        context->PSSetSamplers(0, (UINT)descriptors.samplers.size(), descriptors.samplers.data());
    }

    template <>
    void bindStage<ID3D11ComputeShader>(ID3D11DeviceContext *context, ID3D11ComputeShader *shader, const DescriptorSet &descriptors)
    {
        context->CSSetShader(shader, nullptr, 0);
        context->CSSetConstantBuffers(2, (UINT)descriptors.constants.size(), descriptors.constants.data());
        context->CSSetShaderResources(0, (UINT)descriptors.resources.size(), descriptors.resources.data());
        context->CSSetUnorderedAccessViews(0, (UINT)descriptors.unorderedResources.size(), descriptors.unorderedResources.data(), nullptr);
        context->CSSetSamplers(0, (UINT)descriptors.samplers.size(), descriptors.samplers.data());
	}
}

Batch::Batch(const std::string &name)
    : name(name)
{
}

Job *Batch::addJob()
{
    Job *job = new Job();
    this->jobs.push_back(job);
    return job;
}

void Batch::execute(ID3D11DeviceContext *context)
{
    GPUProfiler::ScopedProfile profile(this->name);

    // if no shader is bound, skip the draw calls
    if (!this->pipeline.vertexShader && !this->pipeline.pixelShader && !this->pipeline.computeShader)
        return;

    if (this->pipeline.depthStencil != nullptr)
        context->OMSetDepthStencilState(this->pipeline.depthStencil, 0);

    if (this->pipeline.vertexShader != nullptr)
        bindStage(context, this->pipeline.vertexShader, this->descriptors);

    if (this->pipeline.pixelShader != nullptr)
        bindStage(context, this->pipeline.pixelShader, this->descriptors);
    
    if (this->pipeline.computeShader != nullptr)
        bindStage(context, this->pipeline.computeShader, this->descriptors);

    if (this->pipeline.inputLayout != nullptr)
        context->IASetInputLayout(this->pipeline.inputLayout);

    // render jobs
    for (auto *job : this->jobs)
    {
        job->execute(context);
        delete job;
    }

    // nullify all resources to unbind them
    this->descriptors.nullify();

	if (this->pipeline.depthStencil != nullptr)
        context->OMSetDepthStencilState(nullptr, 0);

    if (this->pipeline.vertexShader != nullptr)
        bindStage<ID3D11VertexShader>(context, nullptr, this->descriptors);

    if (this->pipeline.pixelShader != nullptr)
        bindStage<ID3D11PixelShader>(context, nullptr, this->descriptors);

    if (this->pipeline.computeShader != nullptr)
        bindStage<ID3D11ComputeShader>(context, nullptr, this->descriptors);

    if (this->pipeline.inputLayout != nullptr)
        context->IASetInputLayout(nullptr);
}
