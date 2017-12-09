#pragma once

#include <string>
#include <vector>

#include <d3d11.h>

class Job;

class Batch
{
    public:
        Batch(const std::string &name);

        void setDepthStencil(ID3D11DepthStencilState *depthStencil) { this->depthStencil = depthStencil; }

        void setResources(const std::vector<ID3D11ShaderResourceView *> &resources) { this->resources = resources; }
		void setUnorderedResources(const std::vector<ID3D11UnorderedAccessView *> &resources) { this->unorderedResources = resources; }
		void setSamplers(const std::vector<ID3D11SamplerState *> &samplers) { this->samplers = samplers; }
		void setShaderConstants(ID3D11Buffer *shaderConstantBuffer) { this->shaderConstantBuffer = shaderConstantBuffer; }

        void setVertexShader(ID3D11VertexShader *vertexShader) { this->vertexShader = vertexShader; }
        void setPixelShader(ID3D11PixelShader *pixelShader) { this->pixelShader = pixelShader; }
        void setComputeShader(ID3D11ComputeShader *computeShader) { this->computeShader = computeShader; }

        void setInputLayout(ID3D11InputLayout *inputLayout) { this->inputLayout = inputLayout; }

        Job *addJob();

        void execute(ID3D11DeviceContext *context);

    private:
        std::string name;

        ID3D11DepthStencilState *depthStencil = nullptr;
        std::vector<ID3D11ShaderResourceView *> resources;
		std::vector<ID3D11UnorderedAccessView *> unorderedResources;
		std::vector<ID3D11SamplerState *> samplers;
		ID3D11Buffer *shaderConstantBuffer = nullptr;

        ID3D11VertexShader *vertexShader = nullptr;
        ID3D11PixelShader *pixelShader = nullptr;
        ID3D11ComputeShader *computeShader = nullptr;

        ID3D11InputLayout *inputLayout = nullptr;

        std::vector<Job *> jobs;
};
