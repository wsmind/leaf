#include <engine/render/ShadowRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/RenderList.h>
#include <engine/scene/Scene.h>

#include <shaders/depthonly.vs.hlsl.h>
#include <shaders/depthonly.ps.hlsl.h>

#pragma pack(push)
#pragma pack(16)
struct DepthOnlyConstants
{
    glm::mat4 transformMatrix;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct ShadowConstants
{
    glm::mat4 lightMatrix[16];
};
#pragma pack(pop)

ShadowRenderer::ShadowRenderer(int resolution)
{
    HRESULT res;

    this->resolution = resolution;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Width = resolution;// *4;
    shadowMapDesc.Height = resolution;// *4;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    res = Device::device->CreateTexture2D(&shadowMapDesc, NULL, &this->shadowMap);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC targetDesc;
    ZeroMemory(&targetDesc, sizeof(targetDesc));
    targetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    targetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(this->shadowMap, &targetDesc, &this->target);
    CHECK_HRESULT(res);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    res = Device::device->CreateShaderResourceView(this->shadowMap, &srvDesc, &this->srv);
    CHECK_HRESULT(res);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    res = Device::device->CreateSamplerState(&samplerDesc, &this->sampler);
    CHECK_HRESULT(res);
    D3D11_DEPTH_STENCIL_DESC depthStateDesc;

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));

    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    res = Device::device->CreateDepthStencilState(&depthStateDesc, &this->depthState);
    CHECK_HRESULT(res);

    res = Device::device->CreateVertexShader(depthonlyVS, sizeof(depthonlyVS), NULL, &this->depthOnlyVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(depthonlyPS, sizeof(depthonlyPS), NULL, &this->depthOnlyPixelShader); CHECK_HRESULT(res);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(DepthOnlyConstants);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->cbDepthOnly);
    CHECK_HRESULT(res);

    cbDesc.ByteWidth = sizeof(ShadowConstants);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->cbShadows);
    CHECK_HRESULT(res);
}

ShadowRenderer::~ShadowRenderer()
{
    this->shadowMap->Release();
    this->target->Release();
    this->srv->Release();
    this->sampler->Release();
    this->depthState->Release();

    this->depthOnlyVertexShader->Release();
    this->depthOnlyPixelShader->Release();

    this->cbDepthOnly->Release();
    this->cbShadows->Release();
}

void ShadowRenderer::render(const Scene *scene, const RenderList *renderList)
{
    Device::context->OMSetRenderTargets(0, nullptr, this->target);
    const std::vector<RenderList::Job> &jobs = renderList->getJobs();
    const std::vector<RenderList::Light> &lights = renderList->getLights();

    Device::context->ClearDepthStencilView(this->target, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_MAPPED_SUBRESOURCE mappedShadowResource;
    HRESULT res = Device::context->Map(this->cbShadows, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedShadowResource);
    CHECK_HRESULT(res);
    ShadowConstants *shadowConstants = (ShadowConstants *)mappedShadowResource.pData;

    int shadowCount = 0;
    for (int i = 0; i < lights.size(); i++)
    {
        // only spotlights cast shadows
        if (!lights[i].spot || shadowCount >= 16)
            continue;

        int index = shadowCount++;

        // apply NDC [-1, 1] to texture space [0, 1] to atlas rect
        /*glm::vec3 scale = glm::vec3(0.5f, 0.5f, 1.0f);
        glm::vec3 offset = glm::vec3(0.5f, 0.5f, 0.0f);
        //glm::vec3 scale = glm::vec3(0.125f, 0.125f, 1.0f);
        //glm::vec3 offset = glm::vec3(0.125f + 0.25f * (float)(index % 4), 0.125f + 0.25f * (float)(index / 4), 0.0f);
        glm::mat4 biasMatrix(
            scale.x, 0.0f, 0.0f, offset.x,
            0.0f, scale.y, 0.0f, offset.y,
            0.0f, 0.0f, scale.z, offset.z,
            0.0f, 0.0f, 0.0f, 1.0f
        );*/
        shadowConstants->lightMatrix[index] = lights[i].shadowTransform;

        GPUProfiler::ScopedProfile profile("Shadow");

        D3D11_VIEWPORT viewport;
        viewport.Width = (float)this->resolution;
        viewport.Height = (float)this->resolution;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f; // (float)((index % 4) * this->resolution);
        viewport.TopLeftY = 0.0f; // (float)((3 - (index / 4)) * this->resolution);
        Device::context->RSSetViewports(1, &viewport);

        Device::context->OMSetDepthStencilState(this->depthState, 0);

        Device::context->VSSetShader(this->depthOnlyVertexShader, NULL, 0);
        Device::context->PSSetShader(this->depthOnlyPixelShader, NULL, 0);

        Mesh * currentMesh = nullptr;
        for (const auto &job : jobs)
        {
            if (currentMesh != job.mesh)
            {
                currentMesh = job.mesh;
                currentMesh->bind();
            }

            D3D11_MAPPED_SUBRESOURCE mappedResource;
            HRESULT res = Device::context->Map(this->cbDepthOnly, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            CHECK_HRESULT(res);
            DepthOnlyConstants *constants = (DepthOnlyConstants *)mappedResource.pData;
            constants->transformMatrix = lights[i].shadowTransform * job.transform;
            Device::context->Unmap(this->cbDepthOnly, 0);

            ID3D11Buffer *allConstantBuffers[] = { this->cbDepthOnly };
            Device::context->VSSetConstantBuffers(0, 1, allConstantBuffers);
            Device::context->PSSetConstantBuffers(0, 1, allConstantBuffers);

            Device::context->DrawIndexed(currentMesh->getIndexCount(), 0, 0);
        }
    }

    Device::context->Unmap(this->cbShadows, 0);
    Device::context->OMSetRenderTargets(0, nullptr, nullptr);
}

void ShadowRenderer::bind()
{
    Device::context->VSSetConstantBuffers(4, 1, &this->cbShadows);
    Device::context->PSSetConstantBuffers(4, 1, &this->cbShadows);

    Device::context->PSSetShaderResources(4, 1, &this->srv);
    Device::context->PSSetSamplers(4, 1, &this->sampler);
}

void ShadowRenderer::unbind()
{
    ID3D11Buffer *nullConstants = nullptr;
    Device::context->VSSetConstantBuffers(4, 1, &nullConstants);
    Device::context->PSSetConstantBuffers(4, 1, &nullConstants);

    ID3D11SamplerState *nullSampler = nullptr;
    ID3D11ShaderResourceView *nullSRV = nullptr;
    Device::context->PSSetSamplers(4, 1, &nullSampler);
    Device::context->PSSetShaderResources(4, 1, &nullSRV);
}
