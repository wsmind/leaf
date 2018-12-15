#include <engine/render/Shaders.h>

#include <engine/render/Device.h>

#include <slang.h>

#include <shaders/streams/depthonly.vs.hlsl.h>
#include <shaders/streams/distancefield.vs.hlsl.h>
#include <shaders/streams/geometry2d.vs.hlsl.h>
#include <shaders/streams/instancedmesh.vs.hlsl.h>

// vertex
#include <shaders/motionblur.vs.hlsl.h>

// pixel
#include <shaders/motionblur.ps.hlsl.h>

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

    D3D11_INPUT_ELEMENT_DESC sdfLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "MODELMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIX", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIXINVERSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIXINVERSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIXINVERSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "MODELMATRIXINVERSE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIX", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIXINVERSE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 176, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIXINVERSE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 192, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "NORMALMATRIXINVERSE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 208, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    res = Device::device->CreateInputLayout(sdfLayout, 18, distanceFieldVS, sizeof(distanceFieldVS), &Shaders::layout.distanceField);
    CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC geometry2DLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    res = Device::device->CreateInputLayout(geometry2DLayout, 4, geometry2DVS, sizeof(geometry2DVS), &Shaders::layout.geometry2D);
    CHECK_HRESULT(res);

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

    res = Device::device->CreateVertexShader(motionblurVS, sizeof(motionblurVS), NULL, &vertex.motionBlur); CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(motionblurPS, sizeof(motionblurPS), NULL, &pixel.motionBlur); CHECK_HRESULT(res);

    res = Device::device->CreateComputeShader(tileMaxCS, sizeof(tileMaxCS), NULL, &compute.tileMax); CHECK_HRESULT(res);
    res = Device::device->CreateComputeShader(neighborMaxCS, sizeof(neighborMaxCS), NULL, &compute.neighborMax); CHECK_HRESULT(res);
    res = Device::device->CreateComputeShader(generateIblCS, sizeof(generateIblCS), NULL, &compute.generateIbl); CHECK_HRESULT(res);
}

void Shaders::unloadShaders()
{
    Shaders::layout.depthOnly->Release();
    Shaders::layout.distanceField->Release();
    Shaders::layout.geometry2D->Release();
    Shaders::layout.instancedMesh->Release();

    vertex.motionBlur->Release();

    pixel.motionBlur->Release();

    compute.tileMax->Release();
    compute.neighborMax->Release();
    compute.generateIbl->Release();
}
