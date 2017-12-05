#include <engine/render/MotionBlurRenderer.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/RenderTarget.h>

#include <shaders/motionblur.ps.hlsl.h>
#include <shaders/tilemax.cs.hlsl.h>

MotionBlurRenderer::MotionBlurRenderer(int backbufferWidth, int backbufferHeight, int tileSize)
{
    this->tileSize = tileSize;
    this->tileCountX = backbufferWidth / tileSize;
    this->tileCountY = backbufferHeight / tileSize;

    HRESULT res;
    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &this->motionblurPixelShader); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &this->tileMaxComputeShader); CHECK_HRESULT(res);

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));
    textureDesc.Width = this->tileCountX;
    textureDesc.Height = this->tileCountY;
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

    ID3D11SamplerState *nullSamplers[] = { nullptr, nullptr, nullptr };
    ID3D11ShaderResourceView *nullSRVs[] = { nullptr, nullptr, nullptr };
    ID3D11UnorderedAccessView *nullUAVs[] = { nullptr, nullptr, nullptr };

    ID3D11RenderTargetView *outputTargetView = outputTarget->getTarget();
    Device::context->OMSetRenderTargets(1, &outputTargetView, nullptr);

    Device::context->PSSetShader(this->motionblurPixelShader, nullptr, 0);

    ID3D11ShaderResourceView *motionSRV = motionTarget->getSRV();
    Device::context->CSSetShaderResources(0, 1, &motionSRV);
    Device::context->CSSetUnorderedAccessViews(0, 1, &this->tileMaxUAV, nullptr);
    Device::context->CSSetShader(this->tileMaxComputeShader, nullptr, 0);
    Device::context->Dispatch(this->tileCountX, this->tileCountY, 1);
    Device::context->CSSetShaderResources(0, 1, nullSRVs);
    Device::context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullptr);

    ID3D11SamplerState *samplerStates[] = { radianceTarget->getSamplerState(), motionTarget->getSamplerState() };
    ID3D11ShaderResourceView *srvs[] = { radianceTarget->getSRV(), motionTarget->getSRV(), this->tileMaxSRV };
    Device::context->PSSetSamplers(0, 2, samplerStates);
    Device::context->PSSetShaderResources(0, 3, srvs);

    Device::context->DrawIndexed(6, 0, 0);

    Device::context->PSSetSamplers(0, 2, nullSamplers);
    Device::context->PSSetShaderResources(0, 3, nullSRVs);
}
