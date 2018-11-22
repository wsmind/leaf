#pragma once

#include <d3d11.h>

struct PipelineLayout
{
    ID3D11VertexShader *vertexShader = nullptr;
    ID3D11PixelShader *pixelShader = nullptr;
    ID3D11ComputeShader *computeShader = nullptr;
};

class ShaderVariant
{
    public:
        ShaderVariant();
        ~ShaderVariant();

        const PipelineLayout &getLayout() const { return this->layout; }

    private:
        PipelineLayout layout;
};
