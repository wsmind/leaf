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
        void setShaderConstants(ID3D11Buffer *shaderConstantBuffer) { this->shaderConstantBuffer = shaderConstantBuffer; }

        void setVertexShader(ID3D11VertexShader *vertexShader) { this->vertexShader = vertexShader; }
        void setPixelShader(ID3D11PixelShader *pixelShader) { this->pixelShader = pixelShader; }
        void setComputeShader(ID3D11ComputeShader *computeShader) { this->computeShader = computeShader; }

        void setInputLayout(ID3D11InputLayout *inputLayout, int vertexStride, int instanceStride)
        {
            this->inputLayout = inputLayout;
            this->vertexStride = vertexStride;
            this->instanceStride = instanceStride;
        }

        Job *addJob();

        void execute(ID3D11DeviceContext *context);

    private:
        std::string name;

        ID3D11DepthStencilState *depthStencil = nullptr;
        std::vector<ID3D11ShaderResourceView *> resources;
        ID3D11Buffer *shaderConstantBuffer = nullptr;

        ID3D11VertexShader *vertexShader = nullptr;
        ID3D11PixelShader *pixelShader = nullptr;
        ID3D11ComputeShader *computeShader = nullptr;

        ID3D11InputLayout *inputLayout = nullptr;
        int vertexStride = 0;
        int instanceStride = 0;

        std::vector<Job *> jobs;
};
