#include <engine/ShadowRenderer.h>

#include <engine/Device.h>

ShadowRenderer::ShadowRenderer(int resolution)
{
    HRESULT res;

    D3D11_TEXTURE2D_DESC shadowMapDesc;
    ZeroMemory(&shadowMapDesc, sizeof(shadowMapDesc));
    shadowMapDesc.Width = resolution;
    shadowMapDesc.Height = resolution;
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
}

ShadowRenderer::~ShadowRenderer()
{
    this->shadowMap->Release();
    this->target->Release();
}

void ShadowRenderer::render(const Scene *scene)
{
    Device::context->OMSetRenderTargets(0, nullptr, this->target);
}
