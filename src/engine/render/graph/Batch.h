#pragma once

#include <string>
#include <vector>

#include <d3d11.h>

class Job;

struct Pipeline
{
    ID3D11DepthStencilState *depthStencil = nullptr;
    ID3D11InputLayout *inputLayout = nullptr;

    ID3D11VertexShader *vertexShader = nullptr;
    ID3D11PixelShader *pixelShader = nullptr;
    ID3D11ComputeShader *computeShader = nullptr;
};

struct DescriptorSet
{
    std::vector<ID3D11ShaderResourceView *> resources;
    std::vector<ID3D11UnorderedAccessView *> unorderedResources;
    std::vector<ID3D11SamplerState *> samplers;
    std::vector<ID3D11Buffer *> constants;

    void append(const DescriptorSet &set)
    {
        this->resources.insert(this->resources.end(), set.resources.begin(), set.resources.end());
        this->unorderedResources.insert(this->unorderedResources.end(), set.unorderedResources.begin(), set.unorderedResources.end());
        this->samplers.insert(this->samplers.end(), set.samplers.begin(), set.samplers.end());
        this->constants.insert(this->constants.end(), set.constants.begin(), set.constants.end());
    }

    void nullify()
    {
        memset(this->resources.data(), 0, sizeof(ID3D11ShaderResourceView *) * this->resources.size());
        memset(this->unorderedResources.data(), 0, sizeof(ID3D11UnorderedAccessView *) * this->unorderedResources.size());
        memset(this->samplers.data(), 0, sizeof(ID3D11SamplerState *) * this->samplers.size());
        memset(this->constants.data(), 0, sizeof(ID3D11Buffer *) * this->constants.size());
    }
};

class Batch
{
    public:
        Batch(const std::string &name);

        void setPipeline(const Pipeline &pipeline) { this->pipeline = pipeline; }
        void setDescriptorSets(const std::vector<DescriptorSet> &sets) { for (const auto &set: sets) { this->descriptors.append(set); } }

        Job *addJob();

        void execute(ID3D11DeviceContext *context);

        // legacy start
        void setDepthStencil(ID3D11DepthStencilState *depthStencil) { this->pipeline.depthStencil = depthStencil; }
        void setInputLayout(ID3D11InputLayout *inputLayout) { this->pipeline.inputLayout = inputLayout; }
        void setVertexShader(ID3D11VertexShader *vertexShader) { this->pipeline.vertexShader = vertexShader; }
        void setPixelShader(ID3D11PixelShader *pixelShader) { this->pipeline.pixelShader = pixelShader; }
        void setComputeShader(ID3D11ComputeShader *computeShader) { this->pipeline.computeShader = computeShader; }

        void setResources(const std::vector<ID3D11ShaderResourceView *> &resources) { this->descriptors.resources = resources; }
        void setUnorderedResources(const std::vector<ID3D11UnorderedAccessView *> &resources) { this->descriptors.unorderedResources = resources; }
        void setSamplers(const std::vector<ID3D11SamplerState *> &samplers) { this->descriptors.samplers = samplers; }
        void setShaderConstants(ID3D11Buffer *shaderConstantBuffer) { this->descriptors.constants.push_back(shaderConstantBuffer); }
        // legacy end

    private:
        std::string name;

        Pipeline pipeline;
        DescriptorSet descriptors;

        std::vector<Job *> jobs;
};
