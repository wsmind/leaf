#include <engine/render/graph/Batch.h>

#include <cassert>

#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/graph/Job.h>

namespace
{
    // base implementation left intentionally undefined; will break the build if an unknown
    // shader type is encountered
    template <class StageType>
    void bindStage(ID3D11DeviceContext *context, StageType *shader, const std::vector<ID3D11ShaderResourceView *> &resources, ID3D11Buffer *shaderConstantBuffer);

    template <>
    void bindStage<ID3D11VertexShader>(ID3D11DeviceContext *context, ID3D11VertexShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, ID3D11Buffer *shaderConstantBuffer)
    {
        context->VSSetShader(shader, nullptr, 0);
        context->VSSetConstantBuffers(2, 1, &shaderConstantBuffer);

        if (resources.size() > 0)
            context->VSSetShaderResources(0, (UINT)resources.size(), &resources[0]);
    }

    template <>
    void bindStage<ID3D11PixelShader>(ID3D11DeviceContext *context, ID3D11PixelShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, ID3D11Buffer *shaderConstantBuffer)
    {
        context->PSSetShader(shader, nullptr, 0);
        context->PSSetConstantBuffers(2, 1, &shaderConstantBuffer);

        if (resources.size() > 0)
            context->PSSetShaderResources(0, (UINT)resources.size(), &resources[0]);
    }

    template <>
    void bindStage<ID3D11ComputeShader>(ID3D11DeviceContext *context, ID3D11ComputeShader *shader, const std::vector<ID3D11ShaderResourceView *> &resources, ID3D11Buffer *shaderConstantBuffer)
    {
        context->CSSetShader(shader, nullptr, 0);
        context->CSSetConstantBuffers(2, 1, &shaderConstantBuffer);

        if (resources.size() > 0)
            context->CSSetShaderResources(0, (UINT)resources.size(), &resources[0]);
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
        bindStage(context, this->vertexShader, this->resources, this->shaderConstantBuffer);

    if (this->pixelShader != nullptr)
        bindStage(context, this->pixelShader, this->resources, this->shaderConstantBuffer);
    
    if (this->computeShader != nullptr)
        bindStage(context, this->computeShader, this->resources, this->shaderConstantBuffer);

    if (this->inputLayout != nullptr)
        context->IASetInputLayout(this->inputLayout);

    // render jobs
    for (auto *job : this->jobs)
    {
        job->execute(context);
        delete job;
    }

    // nullify all resources to unbind them
    if (resources.size() > 0)
        memset(&this->resources[0], 0, sizeof(this->resources[0]) * this->resources.size());

    if (this->depthStencil != nullptr)
        context->OMSetDepthStencilState(nullptr, 0);

    if (this->vertexShader != nullptr)
        bindStage<ID3D11VertexShader>(context, nullptr, this->resources, nullptr);

    if (this->pixelShader != nullptr)
        bindStage<ID3D11PixelShader>(context, nullptr, this->resources, nullptr);

    if (this->computeShader != nullptr)
        bindStage<ID3D11ComputeShader>(context, nullptr, this->resources, nullptr);

    if (this->inputLayout != nullptr)
        context->IASetInputLayout(nullptr);
}
