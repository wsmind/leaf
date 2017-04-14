#include <engine/ShadowRenderer.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/RenderList.h>
#include <engine/Scene.h>

#include <shaders/depthonly.vs.hlsl.h>
#include <shaders/depthonly.ps.hlsl.h>

#pragma pack(push)
#pragma pack(16)
struct DepthOnlyConstants
{
    glm::mat4 transformMatrix;
};
#pragma pack(pop)

ShadowRenderer::ShadowRenderer(int resolution)
{
    HRESULT res;

    this->resolution = resolution;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Width = resolution * 4;
    shadowMapDesc.Height = resolution * 4;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    res = Device::device->CreateTexture2D(&shadowMapDesc, NULL, &this->shadowMap);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC targetDesc;
    ZeroMemory(&targetDesc, sizeof(targetDesc));
    targetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    targetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    targetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(this->shadowMap, &targetDesc, &this->target);
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
}

ShadowRenderer::~ShadowRenderer()
{
    this->shadowMap->Release();
    this->target->Release();
    this->depthState->Release();

    this->depthOnlyVertexShader->Release();
    this->depthOnlyPixelShader->Release();
}

void ShadowRenderer::render(const Scene *scene, const RenderList *renderList)
{
    Device::context->OMSetRenderTargets(0, nullptr, this->target);
    const std::vector<RenderList::Job> &jobs = renderList->getJobs();
    const std::vector<RenderList::Light> &lights = renderList->getLights();

    Device::context->ClearDepthStencilView(this->target, D3D11_CLEAR_DEPTH, 1.0f, 0);

    int shadowCount = 0;
    for (int i = 0; i < lights.size(); i++)
    {
        // only spotlights cast shadows
        if (!lights[i].spot || shadowCount >= 16)
            continue;

        int index = shadowCount++;

        GPUProfiler::ScopedProfile profile("Shadow");

        D3D11_VIEWPORT viewport;
        viewport.Width = (float)this->resolution;
        viewport.Height = (float)this->resolution;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = (float)((index % 4) * this->resolution);
        viewport.TopLeftY = (float)((index / 4) * this->resolution);
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

    Device::context->OMSetRenderTargets(0, nullptr, nullptr);
}
