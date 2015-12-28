#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <comdef.h>
#include <mmsystem.h>

#include <d3d11.h>
#include <gl/GL.h>

#include <shaders/plop.vs.hlsl.h>
#include <shaders/plop.ps.hlsl.h>

Engine *Engine::instance = nullptr;

struct SceneState
{
    float time;
    float _padding[3];
};

#define CHECK_HRESULT(hr) \
    if (hr != S_OK) \
    { \
        _com_error err(hr); \
        printf("Error: %s\n", err.ErrorMessage()); \
        assert(hr == S_OK); \
    }

Engine::Engine()
{
    printf("LeafEngine started\n");

    this->width = 1280;
    this->height = 720;
    this->hwnd = CreateWindow("static", "Leaf", WS_POPUP | WS_VISIBLE, 0, 0, this->width, this->height, NULL, NULL, NULL, 0);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    //set buffer dimensions and format
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = this->width;
    swapChainDesc.BufferDesc.Height = this->height;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

    //set refresh rate
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

    //sampling settings
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SampleDesc.Count = 1;

    //output window handle
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.Windowed = true;

    UINT flags = 0;
    #ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &context);
    CHECK_HRESULT(res);

    ID3D11Texture2D *backBuffer;
    res = swapChain->GetBuffer(0, __uuidof(backBuffer), (void **)&backBuffer);
    CHECK_HRESULT(res);

    res = device->CreateRenderTargetView(backBuffer, NULL, &renderTarget);
    CHECK_HRESULT(res);

    context->OMSetRenderTargets(1, &renderTarget, NULL);

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)this->width;
    viewport.Height = (float)this->height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    context->RSSetViewports(1, &viewport);

    res = device->CreateVertexShader(plopVS, sizeof(plopVS), NULL, &vs);
    CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    res = device->CreateInputLayout(layout, 1, plopVS, sizeof(plopVS), &inputLayout);
    CHECK_HRESULT(res);

    res = device->CreatePixelShader(plopPS, sizeof(plopPS), NULL, &ps);
    CHECK_HRESULT(res);

    float vertices[] = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
    };

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.StructureByteStride = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    res = device->CreateBuffer(&vbDesc, &vertexData, &vb);
    CHECK_HRESULT(res)

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(SceneState);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    res = device->CreateBuffer(&cbDesc, NULL, &cb);
    CHECK_HRESULT(res);

    startTime = timeGetTime();
}

Engine::~Engine()
{
    printf("LeafEngine stopped\n");

    DestroyWindow(this->hwnd);
}

void Engine::render()
{
    float time = (float)(timeGetTime() - startTime) * 0.001f * 140.0f / 60.0f;

    float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    context->ClearRenderTargetView(renderTarget, clearColor);

    context->VSSetShader(vs, NULL, 0);
    context->PSSetShader(ps, NULL, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    SceneState *sceneState = (SceneState *)mappedResource.pData;
    sceneState->time = time;
    context->Unmap(cb, 0);
    context->PSSetConstantBuffers(0, 1, &cb);

    context->IASetInputLayout(inputLayout);

    UINT stride = sizeof(float) * 2;
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    context->Draw(4, 0);

    swapChain->Present(0, 0);
}

void Engine::renderBlenderViewport()
{
    this->render();

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
