#include <engine/MotionBlurRenderer.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/RenderTarget.h>

#include <shaders/motionblur.ps.hlsl.h>
#include <shaders/tilemax.cs.hlsl.h>

MotionBlurRenderer::MotionBlurRenderer()
{
    HRESULT res;
    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &this->motionblurPixelShader); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &this->tileMaxComputeShader); CHECK_HRESULT(res);

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = 10;
    textureDesc.Height = 10;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

    res = Device::device->CreateTexture2D(&textureDesc, NULL, &this->tileMaxTexture);
    CHECK_HRESULT(res);

    res = Device::device->CreateShaderResourceView(this->tileMaxTexture, NULL, &this->tileMaxSRV);
    CHECK_HRESULT(res);

    res = Device::device->CreateUnorderedAccessView(this->tileMaxTexture, NULL, &this->tileMaxUAV);
    CHECK_HRESULT(res);
}

MotionBlurRenderer::~MotionBlurRenderer()
{
    this->motionblurPixelShader->Release();
    this->tileMaxComputeShader->Release();

    this->tileMaxTexture->Release();
    this->tileMaxSRV->Release();
    this->tileMaxUAV->Release();
}

void MotionBlurRenderer::render(RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget)
{
    GPUProfiler::ScopedProfile profile("MotionBlur");

    ID3D11RenderTargetView *outputTargetView = outputTarget->getTarget();
    Device::context->OMSetRenderTargets(1, &outputTargetView, nullptr);

    Device::context->PSSetShader(this->motionblurPixelShader, NULL, 0);

    ID3D11SamplerState *samplerStates[] = { radianceTarget->getSamplerState(), motionTarget->getSamplerState() };
    ID3D11ShaderResourceView *srvs[] = { radianceTarget->getSRV(), motionTarget->getSRV() };
    Device::context->PSSetSamplers(0, 2, samplerStates);
    Device::context->PSSetShaderResources(0, 2, srvs);

    Device::context->DrawIndexed(6, 0, 0);

    ID3D11SamplerState *nullSamplers[] = { nullptr, nullptr };
    ID3D11ShaderResourceView *nullSRVs[] = { nullptr, nullptr };
    Device::context->PSSetSamplers(0, 2, nullSamplers);
    Device::context->PSSetShaderResources(0, 2, nullSRVs);
}
