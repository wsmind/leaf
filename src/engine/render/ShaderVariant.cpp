#include <engine/render/ShaderVariant.h>

#include <cstdio>
#include <slang.h>

#include <engine/render/Device.h>

ShaderVariant::~ShaderVariant()
{
    if (this->pipeline.vertexShader != nullptr)
        this->pipeline.vertexShader->Release();

    if (this->pipeline.pixelShader != nullptr)
        this->pipeline.pixelShader->Release();

    if (this->pipeline.computeShader != nullptr)
        this->pipeline.computeShader->Release();
}

void ShaderVariant::compileShaders(SlangCompileRequest *slangRequest, int translationUnitIndex, FILE *exportStream)
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

    // save the compiled blobs when exporting
    if (exportStream != nullptr)
    {
        size_t vertexBlobSize = vertexShaderBlob->getBufferSize();
        fwrite(&vertexBlobSize, sizeof(vertexBlobSize), 1, exportStream);
        fwrite(vertexShaderBlob->getBufferPointer(), vertexBlobSize, 1, exportStream);

        size_t fragmentBlobSize = fragmentShaderBlob->getBufferSize();
        fwrite(&fragmentBlobSize, sizeof(fragmentBlobSize), 1, exportStream);
        fwrite(fragmentShaderBlob->getBufferPointer(), fragmentBlobSize, 1, exportStream);

        size_t computeBlobSize = 0;
        fwrite(&computeBlobSize, sizeof(computeBlobSize), 1, exportStream);
        //fwrite(computeShaderBlob->getBufferPointer(), computeBlobSize, 1, exportStream);
    }
}

void ShaderVariant::loadShaders(FILE *inputStream)
{
    size_t blobSize = 0;
    std::vector<unsigned char> blob;
    HRESULT res;

    fread(&blobSize, sizeof(blobSize), 1, inputStream);
    blob.resize(blobSize);
    fread(blob.data(), blobSize, 1, inputStream);
    res = Device::device->CreateVertexShader(blob.data(), blobSize, NULL, &this->pipeline.vertexShader);
    CHECK_HRESULT(res);

    fread(&blobSize, sizeof(blobSize), 1, inputStream);
    blob.resize(blobSize);
    fread(blob.data(), blobSize, 1, inputStream);
    res = Device::device->CreatePixelShader(blob.data(), blobSize, NULL, &this->pipeline.pixelShader);
    CHECK_HRESULT(res);

    fread(&blobSize, sizeof(blobSize), 1, inputStream);
    blob.resize(blobSize);
    fread(blob.data(), blobSize, 1, inputStream);
    //res = Device::device->CreateComputeShader(blob.data(), blobSize, NULL, &this->pipeline.pixelShader);
    //CHECK_HRESULT(res);
}
