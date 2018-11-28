#include <engine/render/Shaders.h>

#include <engine/render/Device.h>

#include <slang.h>

#include <shaders/streams/instancedmesh.vs.hlsl.h>
#include <shaders/streams/depthonly.vs.hlsl.h>

// vertex
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
LayoutList Shaders::layout;

void Shaders::loadShaders()
{
    HRESULT res;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "MODELMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLDTOPREVIOUSFRAMECLIPSPACE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLDTOPREVIOUSFRAMECLIPSPACE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLDTOPREVIOUSFRAMECLIPSPACE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLDTOPREVIOUSFRAMECLIPSPACE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    res = Device::device->CreateInputLayout(layout, 15, instancedMeshVS, sizeof(instancedMeshVS), &Shaders::layout.instancedMesh);
    CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC depthOnlyLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    res = Device::device->CreateInputLayout(depthOnlyLayout, 8, depthOnlyVS, sizeof(depthOnlyVS), &Shaders::layout.depthOnly);
    CHECK_HRESULT(res);

    res = Device::device->CreateVertexShader(instancedMeshVS, sizeof(instancedMeshVS), NULL, &vertex.basic); CHECK_HRESULT(res);
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
    Shaders::layout.instancedMesh->Release();
    Shaders::layout.depthOnly->Release();

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
