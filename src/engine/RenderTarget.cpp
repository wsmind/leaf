#include <engine/RenderTarget.h>

#include <engine/Device.h>

RenderTarget::RenderTarget(int width, int height, DXGI_FORMAT format, bool msaa)
{
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = format;
    textureDesc.SampleDesc.Count = msaa ? 4 : 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    HRESULT res = Device::device->CreateTexture2D(&textureDesc, NULL, &this->texture);
    CHECK_HRESULT(res);

    res = Device::device->CreateRenderTargetView(this->texture, NULL, &this->target);
    CHECK_HRESULT(res);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    res = Device::device->CreateSamplerState(&samplerDesc, &this->samplerState);
    CHECK_HRESULT(res);

    res = Device::device->CreateShaderResourceView(this->texture, NULL, &this->srv);
    CHECK_HRESULT(res);
}

RenderTarget::~RenderTarget()
{
    this->texture->Release();
    this->target->Release();
    this->samplerState->Release();
    this->srv->Release();
}
