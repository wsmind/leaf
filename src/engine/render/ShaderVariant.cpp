#include <engine/render/ShaderVariant.h>

#include <cstdio>
#include <slang.h>

#include <engine/render/Device.h>

ShaderVariant::ShaderVariant(SlangCompileRequest *slangRequest, int translationUnitIndex)
{
    int vertexIndex = spAddEntryPoint(slangRequest, translationUnitIndex, "vertexMain", SLANG_STAGE_VERTEX);
    int fragmentIndex = spAddEntryPoint(slangRequest, translationUnitIndex, "fragmentMain", SLANG_STAGE_FRAGMENT);

    const SlangResult result = spCompile(slangRequest);

    // display errors and warnings, if any
    if (auto diagnostics = spGetDiagnosticOutput(slangRequest))
    {
        printf("%s", diagnostics);
    }

    if (SLANG_FAILED(result))
        return;

    HRESULT res;

    ISlangBlob* vertexShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, vertexIndex, 0, &vertexShaderBlob);
    res = Device::device->CreateVertexShader(vertexShaderBlob->getBufferPointer(), vertexShaderBlob->getBufferSize(), NULL, &this->layout.vertexShader);
    CHECK_HRESULT(res);

    ISlangBlob* fragmentShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, fragmentIndex, 0, &fragmentShaderBlob);
    res = Device::device->CreatePixelShader(fragmentShaderBlob->getBufferPointer(), fragmentShaderBlob->getBufferSize(), NULL, &this->layout.pixelShader);
    CHECK_HRESULT(res);
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
