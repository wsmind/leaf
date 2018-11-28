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
    //printf("%s\n", (const char *)vertexShaderBlob->getBufferPointer());
    res = Device::device->CreateVertexShader(vertexShaderBlob->getBufferPointer(), vertexShaderBlob->getBufferSize(), NULL, &this->pipeline.vertexShader);
    CHECK_HRESULT(res);

    ISlangBlob* fragmentShaderBlob = nullptr;
    spGetEntryPointCodeBlob(slangRequest, fragmentIndex, 0, &fragmentShaderBlob);
    //printf("%s\n", (const char *)fragmentShaderBlob->getBufferPointer());
    res = Device::device->CreatePixelShader(fragmentShaderBlob->getBufferPointer(), fragmentShaderBlob->getBufferSize(), NULL, &this->pipeline.pixelShader);
    CHECK_HRESULT(res);
}

ShaderVariant::~ShaderVariant()
{
    if (this->pipeline.vertexShader != nullptr)
        this->pipeline.vertexShader->Release();

    if (this->pipeline.pixelShader != nullptr)
        this->pipeline.pixelShader->Release();

    if (this->pipeline.computeShader != nullptr)
        this->pipeline.computeShader->Release();
}
