#include <engine/MotionBlurRenderer.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/RenderTarget.h>

#include <shaders/motionblur.ps.hlsl.h>

MotionBlurRenderer::MotionBlurRenderer()
{
    HRESULT res;
    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &this->motionblurPixelShader); CHECK_HRESULT(res);
}

MotionBlurRenderer::~MotionBlurRenderer()
{
    this->motionblurPixelShader->Release();
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
