#include <engine/render/graph/Batch.h>

#include <cassert>

#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Job.h>

namespace
{
    // base implementation left intentionally undefined; will break the build if an unknown
    // shader type is encountered
    template <class StageType>
    void bindStage(ID3D11DeviceContext *context, StageType *shader, const std::vector<ID3D11ShaderResourceView *> &resources, const std::vector<ID3D11UnorderedAccessView *> &uavs, const std::vector<ID3D11SamplerState *> &samplers, ID3D11Buffer *shaderConstantBuffer);

    template <>
    void bindStage<ID3D11VertexShader>(ID3D11DeviceContext *context, ID3D11VertexShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, const std::vector<ID3D11UnorderedAccessView *> &uavs, const std::vector<ID3D11SamplerState *> &samplers, ID3D11Buffer *shaderConstantBuffer)
    {
        context->VSSetShader(shader, nullptr, 0);
        context->VSSetConstantBuffers(2, 1, &shaderConstantBuffer);
        context->VSSetShaderResources(0, (UINT)resources.size(), resources.data());
        context->VSSetSamplers(0, (UINT)samplers.size(), samplers.data());
    }

    template <>
    void bindStage<ID3D11PixelShader>(ID3D11DeviceContext *context, ID3D11PixelShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, const std::vector<ID3D11UnorderedAccessView *> &uavs, const std::vector<ID3D11SamplerState *> &samplers, ID3D11Buffer *shaderConstantBuffer)
    {
        context->PSSetShader(shader, nullptr, 0);
        context->PSSetConstantBuffers(2, 1, &shaderConstantBuffer);
        context->PSSetShaderResources(0, (UINT)resources.size(), resources.data());
        context->PSSetSamplers(0, (UINT)samplers.size(), samplers.data());
	}

    template <>
    void bindStage<ID3D11ComputeShader>(ID3D11DeviceContext *context, ID3D11ComputeShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, const std::vector<ID3D11UnorderedAccessView *> &uavs, const std::vector<ID3D11SamplerState *> &samplers, ID3D11Buffer *shaderConstantBuffer)
    {
        context->CSSetShader(shader, nullptr, 0);
        context->CSSetConstantBuffers(2, 1, &shaderConstantBuffer);
        context->CSSetShaderResources(0, (UINT)resources.size(), resources.data());
		context->CSSetUnorderedAccessViews(0, (UINT)uavs.size(), uavs.data(), nullptr);
		context->CSSetSamplers(0, (UINT)samplers.size(), samplers.data());
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

    if (this->depthStencil != nullptr)
        context->OMSetDepthStencilState(this->depthStencil, 0);

    if (this->vertexShader != nullptr)
        bindStage(context, this->vertexShader, this->resources, this->unorderedResources, this->samplers, this->shaderConstantBuffer);

    if (this->pixelShader != nullptr)
        bindStage(context, this->pixelShader, this->resources, this->unorderedResources, this->samplers, this->shaderConstantBuffer);
    
    if (this->computeShader != nullptr)
        bindStage(context, this->computeShader, this->resources, this->unorderedResources, this->samplers, this->shaderConstantBuffer);

    if (this->inputLayout != nullptr)
        context->IASetInputLayout(this->inputLayout);

    // render jobs
    for (auto *job : this->jobs)
    {
        job->execute(context);
        delete job;
    }

    // nullify all resources to unbind them
    if (this->resources.size() > 0)
        memset(&this->resources[0], 0, sizeof(this->resources[0]) * this->resources.size());

	if (this->unorderedResources.size() > 0)
		memset(&this->unorderedResources[0], 0, sizeof(this->unorderedResources[0]) * this->unorderedResources.size());

	if (this->samplers.size() > 0)
		memset(&this->samplers[0], 0, sizeof(this->samplers[0]) * this->samplers.size());

	if (this->depthStencil != nullptr)
        context->OMSetDepthStencilState(nullptr, 0);

    if (this->vertexShader != nullptr)
        bindStage<ID3D11VertexShader>(context, nullptr, this->resources, this->unorderedResources, this->samplers, nullptr);

    if (this->pixelShader != nullptr)
        bindStage<ID3D11PixelShader>(context, nullptr, this->resources, this->unorderedResources, this->samplers, nullptr);

    if (this->computeShader != nullptr)
        bindStage<ID3D11ComputeShader>(context, nullptr, this->resources, this->unorderedResources, this->samplers, nullptr);

    if (this->inputLayout != nullptr)
        context->IASetInputLayout(nullptr);
}
