#include <engine/render/PostProcessor.h>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Mesh.h>
#include <engine/render/MotionBlurRenderer.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Pass.h>
#include <engine/resource/ResourceManager.h>

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

void PostProcessor::render(FrameGraph *frameGraph, int width, int height, RenderTarget *motionTarget)
{
    this->fullscreenQuad->bind();

    //Device::context->VSSetShader(postprocessVertexShader, NULL, 0);

    //this->motionBlurRenderer->render(this->targets[0], motionTarget, this->targets[1]);

    ID3D11RenderTargetView *target0 = this->targets[0]->getTarget();
    ID3D11RenderTargetView *target1 = this->targets[1]->getTarget();

    // tone mapping and gamma correction
    Pass *toneMappingPass = frameGraph->addPass("ToneMapping");
    toneMappingPass->setTargets({ this->targets[0]->getTarget() }, nullptr);
    // need viewport

    Batch *toneMappingBatch = toneMappingPass->addBatch("");
    toneMappingBatch->setResources({ this->targets[1]->getSRV() });
    toneMappingBatch->setVertexShader(postprocessVertexShader);
    toneMappingBatch->setPixelShader(postprocessPixelShader);

    Job *toneMappingJob = toneMappingBatch->addJob();
    this->fullscreenQuad->setupJob(toneMappingJob);

    //Device::context->OMSetRenderTargets(1, &target0, nullptr);

    //Device::context->PSSetShader(postprocessPixelShader, NULL, 0);

    //ID3D11SamplerState *radianceSamplerState = this->targets[1]->getSamplerState();
    //ID3D11ShaderResourceView *radianceSRV = this->targets[1]->getSRV();
    //Device::context->PSSetSamplers(0, 1, &radianceSamplerState);
    //Device::context->PSSetShaderResources(0, 1, &radianceSRV);

    //Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);

    /*ID3D11SamplerState *nullSampler = nullptr;
    ID3D11ShaderResourceView *nullSRV = nullptr;
    Device::context->PSSetSamplers(0, 1, &nullSampler);
    Device::context->PSSetShaderResources(0, 1, &nullSRV);*/

    // fxaa pass and blit to backbuffer
    Pass *fxaaPass = frameGraph->addPass("FXAA");
    fxaaPass->setTargets({ this->backBufferTarget }, nullptr);

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    fxaaPass->setViewport(viewport, glm::mat4(), glm::mat4());

    Batch *fxaaBatch = toneMappingPass->addBatch("");
    fxaaBatch->setResources({ this->targets[0]->getSRV() });
    fxaaBatch->setVertexShader(fxaaVertexShader);
    fxaaBatch->setPixelShader(fxaaPixelShader);

    Job *fxaaJob = fxaaBatch->addJob();
    this->fullscreenQuad->setupJob(fxaaJob);

    //ID3D11SamplerState *target1SamplerState = this->targets[0]->getSamplerState();
    //Device::context->PSSetSamplers(0, 1, &target1SamplerState);

    //Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);
}
