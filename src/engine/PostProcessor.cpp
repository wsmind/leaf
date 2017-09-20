#include <engine/PostProcessor.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/Mesh.h>
#include <engine/MotionBlurRenderer.h>
#include <engine/RenderTarget.h>
#include <engine/ResourceManager.h>

#include <shaders/fxaa.vs.hlsl.h>
#include <shaders/fxaa.ps.hlsl.h>
#include <shaders/postprocess.vs.hlsl.h>
#include <shaders/postprocess.ps.hlsl.h>

PostProcessor::PostProcessor(ID3D11RenderTargetView *backBufferTarget)
{
    this->backBufferTarget = backBufferTarget;

    ID3D11Resource *backBufferResource;
    ID3D11Texture2D *backBufferTexture;
    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBufferTarget->GetResource(&backBufferResource);
    backBufferResource->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID *)&backBufferTexture);
    backBufferTexture->GetDesc(&backBufferDesc);

    HRESULT res;
    res = Device::device->CreateVertexShader(postprocessVS, sizeof(postprocessVS), NULL, &postprocessVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(postprocessPS, sizeof(postprocessPS), NULL, &postprocessPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(fxaaVS, sizeof(fxaaVS), NULL, &fxaaVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(fxaaPS, sizeof(fxaaPS), NULL, &fxaaPixelShader); CHECK_HRESULT(res);

    this->targets[0] = new RenderTarget(backBufferDesc.Width, backBufferDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT);
    this->targets[1] = new RenderTarget(backBufferDesc.Width, backBufferDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");

    this->motionBlurRenderer = new MotionBlurRenderer(backBufferDesc.Width, backBufferDesc.Height, 40);
}

PostProcessor::~PostProcessor()
{
    this->postprocessVertexShader->Release();
    this->postprocessPixelShader->Release();
    this->fxaaVertexShader->Release();
    this->fxaaPixelShader->Release();

    delete this->targets[0];
    delete this->targets[1];

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    delete this->motionBlurRenderer;
}

void PostProcessor::render(int width, int height, RenderTarget *motionTarget)
{
    GPUProfiler::ScopedProfile profile("PostProcess");

    this->fullscreenQuad->bind();

    Device::context->VSSetShader(postprocessVertexShader, NULL, 0);

    this->motionBlurRenderer->render(this->targets[0], motionTarget, this->targets[1]);

    ID3D11RenderTargetView *target0 = this->targets[0]->getTarget();
    ID3D11RenderTargetView *target1 = this->targets[1]->getTarget();

    // tone mapping and gamma correction
    Device::context->OMSetRenderTargets(1, &target0, nullptr);

    Device::context->PSSetShader(postprocessPixelShader, NULL, 0);

    ID3D11SamplerState *radianceSamplerState = this->targets[1]->getSamplerState();
    ID3D11ShaderResourceView *radianceSRV = this->targets[1]->getSRV();
    Device::context->PSSetSamplers(0, 1, &radianceSamplerState);
    Device::context->PSSetShaderResources(0, 1, &radianceSRV);

    Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);

    ID3D11SamplerState *nullSampler = nullptr;
    ID3D11ShaderResourceView *nullSRV = nullptr;
    Device::context->PSSetSamplers(0, 1, &nullSampler);
    Device::context->PSSetShaderResources(0, 1, &nullSRV);

    // fxaa pass and blit to backbuffer

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    Device::context->RSSetViewports(1, &viewport);

    Device::context->OMSetRenderTargets(1, &this->backBufferTarget, nullptr);

    Device::context->VSSetShader(fxaaVertexShader, NULL, 0);
    Device::context->PSSetShader(fxaaPixelShader, NULL, 0);

    ID3D11SamplerState *target1SamplerState = this->targets[0]->getSamplerState();
    ID3D11ShaderResourceView *target1SRV = this->targets[0]->getSRV();
    Device::context->PSSetSamplers(0, 1, &target1SamplerState);
    Device::context->PSSetShaderResources(0, 1, &target1SRV);

    Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);

    Device::context->PSSetSamplers(0, 1, &nullSampler);
    Device::context->PSSetShaderResources(0, 1, &nullSRV);
}
