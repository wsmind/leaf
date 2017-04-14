#include <engine/ShadowRenderer.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/RenderList.h>
#include <engine/Scene.h>

ShadowRenderer::ShadowRenderer(int resolution)
{
    HRESULT res;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Width = resolution;
    shadowMapDesc.Height = resolution;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 16;
    shadowMapDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.SampleDesc.Quality = 0;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    res = Device::device->CreateTexture2D(&shadowMapDesc, NULL, &this->shadowMap);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC targetDesc;
    ZeroMemory(&targetDesc, sizeof(targetDesc));
    targetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    targetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    targetDesc.Texture2DArray.MipSlice = 0;
    targetDesc.Texture2DArray.FirstArraySlice= 0;
    targetDesc.Texture2DArray.ArraySize= 16;

    res = Device::device->CreateDepthStencilView(this->shadowMap, &targetDesc, &this->target);
    CHECK_HRESULT(res);
}

ShadowRenderer::~ShadowRenderer()
{
    this->shadowMap->Release();
    this->target->Release();
}

void ShadowRenderer::render(const Scene *scene, const RenderList *renderList)
{
    Device::context->OMSetRenderTargets(0, nullptr, this->target);
    const std::vector<RenderList::Job> &jobs = renderList->getJobs();

    {
        GPUProfiler::ScopedProfile profile("Shadow");
        Mesh * currentMesh = nullptr;
        for (const auto &job : jobs)
        {
            if (currentMesh != job.mesh)
            {
                currentMesh = job.mesh;
                currentMesh->bind();
            }

            /*HRESULT res = Device::context->Map(this->cbInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            CHECK_HRESULT(res);
            InstanceData *instanceData = (InstanceData *)mappedResource.pData;
            instanceData->modelMatrix = job.transform;
            Device::context->Unmap(this->cbInstance, 0);*/

            Device::context->DrawIndexed(currentMesh->getIndexCount(), 0, 0);
        }
    }

    Device::context->OMSetRenderTargets(0, nullptr, nullptr);
}
