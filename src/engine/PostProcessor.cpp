#include <engine/PostProcessor.h>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/Mesh.h>
#include <engine/RenderTarget.h>
#include <engine/ResourceManager.h>

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

    this->radianceTarget = new RenderTarget(backBufferDesc.Width, backBufferDesc.Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");
}

PostProcessor::~PostProcessor()
{
    this->postprocessVertexShader->Release();
    this->postprocessPixelShader->Release();

    delete this->radianceTarget;

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);
}

void PostProcessor::render(int width, int height)
{
    GPUProfiler::ScopedProfile profile("PostProcess");

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    Device::context->RSSetViewports(1, &viewport);

    Device::context->OMSetRenderTargets(1, &this->backBufferTarget, nullptr);

    Device::context->VSSetShader(postprocessVertexShader, NULL, 0);
    Device::context->PSSetShader(postprocessPixelShader, NULL, 0);

    ID3D11SamplerState *radianceSamplerState = this->radianceTarget->getSamplerState();
    ID3D11ShaderResourceView *radianceSRV = this->radianceTarget->getSRV();
    Device::context->PSSetSamplers(0, 1, &radianceSamplerState);
    Device::context->PSSetShaderResources(0, 1, &radianceSRV);

    this->fullscreenQuad->bind();
    Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);
}
