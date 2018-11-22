#include <engine/render/ShaderVariant.h>

ShaderVariant::ShaderVariant()
{
}

ShaderVariant::~ShaderVariant()
{
    if (this->layout.vertexShader != nullptr)
        this->layout.vertexShader->Release();

    if (this->layout.pixelShader != nullptr)
        this->layout.pixelShader->Release();

    if (this->layout.computeShader != nullptr)
        this->layout.computeShader->Release();
}
