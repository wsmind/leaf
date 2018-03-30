#include <engine/render/Shaders.h>

#include <engine/render/Device.h>

// vertex
#include <shaders/background.vs.hlsl.h>
#include <shaders/basic.vs.hlsl.h>
#include <shaders/bloom.vs.hlsl.h>
#include <shaders/depthonly.vs.hlsl.h>
#include <shaders/fxaa.vs.hlsl.h>
#include <shaders/motionblur.vs.hlsl.h>
#include <shaders/plop.vs.hlsl.h>
#include <shaders/postprocess.vs.hlsl.h>
#include <shaders/standard.vs.hlsl.h>
#include <shaders/unlit.vs.hlsl.h>

// pixel
#include <shaders/background.ps.hlsl.h>
#include <shaders/basic.ps.hlsl.h>
#include <shaders/bloomthreshold.ps.hlsl.h>
#include <shaders/bloomdownsample.ps.hlsl.h>
#include <shaders/bloomaccumulation.ps.hlsl.h>
#include <shaders/bloomdebug.ps.hlsl.h>
#include <shaders/depthonly.ps.hlsl.h>
#include <shaders/fxaa.ps.hlsl.h>
#include <shaders/motionblur.ps.hlsl.h>
#include <shaders/plop.ps.hlsl.h>
#include <shaders/postprocess.ps.hlsl.h>
#include <shaders/standard.ps.hlsl.h>
#include <shaders/unlit.ps.hlsl.h>

// compute
#include <shaders/neighbormax.cs.hlsl.h>
#include <shaders/tilemax.cs.hlsl.h>

VertexShaderList Shaders::vertex;
PixelShaderList Shaders::pixel;
ComputeShaderList Shaders::compute;

void Shaders::loadShaders()
{
    HRESULT res;

    res = Device::device->CreateVertexShader(backgroundVS, sizeof(backgroundVS), NULL, &vertex.background); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(basicVS, sizeof(basicVS), NULL, &vertex.basic); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(bloomVS, sizeof(bloomVS), NULL, &vertex.bloom); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(depthonlyVS, sizeof(depthonlyVS), NULL, &vertex.depthOnly); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(fxaaVS, sizeof(fxaaVS), NULL, &vertex.fxaa); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(motionblurVS, sizeof(motionblurVS), NULL, &vertex.motionBlur); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(plopVS, sizeof(plopVS), NULL, &vertex.plop); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(postprocessVS, sizeof(postprocessVS), NULL, &vertex.postprocess); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(standardVS, sizeof(standardVS), NULL, &vertex.standard); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(unlitVS, sizeof(unlitVS), NULL, &vertex.unlit); CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(backgroundPS, sizeof(backgroundPS), NULL, &pixel.background); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(basicPS, sizeof(basicPS), NULL, &pixel.basic); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomThresholdPS, sizeof(bloomThresholdPS), NULL, &pixel.bloomThreshold); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomDownsamplePS, sizeof(bloomDownsamplePS), NULL, &pixel.bloomDownsample); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomAccumulationPS, sizeof(bloomAccumulationPS), NULL, &pixel.bloomAccumulation); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(bloomDebugPS, sizeof(bloomDebugPS), NULL, &pixel.bloomDebug); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(depthonlyPS, sizeof(depthonlyPS), NULL, &pixel.depthOnly); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(fxaaPS, sizeof(fxaaPS), NULL, &pixel.fxaa); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &pixel.motionBlur); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(plopPS, sizeof(plopPS), NULL, &pixel.plop); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(postprocessPS, sizeof(postprocessPS), NULL, &pixel.postprocess); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(standardPS, sizeof(standardPS), NULL, &pixel.standard); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(unlitPS, sizeof(unlitPS), NULL, &pixel.unlit); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &compute.tileMax); CHECK_HRESULT(res);
    res = Device::device->CreateComputeShader(neighborMaxCS, sizeof(neighborMaxCS), NULL, &compute.neighborMax); CHECK_HRESULT(res);
}

void Shaders::unloadShaders()
{
    vertex.background->Release();
    vertex.basic->Release();
    vertex.bloom->Release();
    vertex.depthOnly->Release();
    vertex.fxaa->Release();
    vertex.motionBlur->Release();
    vertex.plop->Release();
    vertex.postprocess->Release();
    vertex.standard->Release();
    vertex.unlit->Release();

    pixel.background->Release();
    pixel.basic->Release();
    pixel.bloomThreshold->Release();
    pixel.bloomDownsample->Release();
    pixel.bloomAccumulation->Release();
    pixel.bloomDebug->Release();
    pixel.depthOnly->Release();
    pixel.fxaa->Release();
    pixel.motionBlur->Release();
    pixel.plop->Release();
    pixel.postprocess->Release();
    pixel.standard->Release();
    pixel.unlit->Release();

    compute.tileMax->Release();
    compute.neighborMax->Release();
}
