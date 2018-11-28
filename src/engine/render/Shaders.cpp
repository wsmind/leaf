#include <engine/render/Shaders.h>

#include <engine/render/Device.h>

#include <slang.h>

// vertex
#include <shaders/basic.vs.hlsl.h>
#include <shaders/bloom.vs.hlsl.h>
#include <shaders/fxaa.vs.hlsl.h>
#include <shaders/motionblur.vs.hlsl.h>
#include <shaders/plop.vs.hlsl.h>

// pixel
#include <shaders/basic.ps.hlsl.h>
#include <shaders/bloomthreshold.ps.hlsl.h>
#include <shaders/bloomdownsample.ps.hlsl.h>
#include <shaders/bloomaccumulation.ps.hlsl.h>
#include <shaders/bloomdebug.ps.hlsl.h>
#include <shaders/fxaa.ps.hlsl.h>
#include <shaders/motionblur.ps.hlsl.h>
#include <shaders/plop.ps.hlsl.h>

// compute
#include <shaders/generateibl.cs.hlsl.h>
#include <shaders/neighbormax.cs.hlsl.h>
#include <shaders/tilemax.cs.hlsl.h>

VertexShaderList Shaders::vertex;
PixelShaderList Shaders::pixel;
ComputeShaderList Shaders::compute;

void Shaders::loadShaders()
{
    SlangSession *slangSession = spCreateSession(nullptr);
    spDestroySession(slangSession);

    HRESULT res;

    res = Device::device->CreateVertexShader(basicVS, sizeof(basicVS), NULL, &vertex.basic); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(bloomVS, sizeof(bloomVS), NULL, &vertex.bloom); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(fxaaVS, sizeof(fxaaVS), NULL, &vertex.fxaa); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(motionblurVS, sizeof(motionblurVS), NULL, &vertex.motionBlur); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(plopVS, sizeof(plopVS), NULL, &vertex.plop); CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(basicPS, sizeof(basicPS), NULL, &pixel.basic); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomThresholdPS, sizeof(bloomThresholdPS), NULL, &pixel.bloomThreshold); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomDownsamplePS, sizeof(bloomDownsamplePS), NULL, &pixel.bloomDownsample); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomAccumulationPS, sizeof(bloomAccumulationPS), NULL, &pixel.bloomAccumulation); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomDebugPS, sizeof(bloomDebugPS), NULL, &pixel.bloomDebug); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(fxaaPS, sizeof(fxaaPS), NULL, &pixel.fxaa); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &pixel.motionBlur); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(plopPS, sizeof(plopPS), NULL, &pixel.plop); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &compute.tileMax); CHECK_HRESULT(res);
    res = Device::device->CreateComputeShader(neighborMaxCS, sizeof(neighborMaxCS), NULL, &compute.neighborMax); CHECK_HRESULT(res);
    res = Device::device->CreateComputeShader(generateIblCS, sizeof(generateIblCS), NULL, &compute.generateIbl); CHECK_HRESULT(res);
}

void Shaders::unloadShaders()
{
    vertex.basic->Release();
    vertex.bloom->Release();
    vertex.fxaa->Release();
    vertex.motionBlur->Release();
    vertex.plop->Release();

    pixel.basic->Release();
    pixel.bloomThreshold->Release();
    pixel.bloomDownsample->Release();
    pixel.bloomAccumulation->Release();
    pixel.bloomDebug->Release();
    pixel.fxaa->Release();
    pixel.motionBlur->Release();
    pixel.plop->Release();

    compute.tileMax->Release();
    compute.neighborMax->Release();
    compute.generateIbl->Release();
}
