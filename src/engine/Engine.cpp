#include <engine/Engine.h>

#include <cstdio>

#include <windows.h>
#include <mmsystem.h>

#include <d3d11.h>
#include <gl/GL.h>

#include <engine/Device.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/ResourceManager.h>

#include <shaders/plop.vs.hlsl.h>
#include <shaders/plop.ps.hlsl.h>

Engine *Engine::instance = nullptr;

struct SceneState
{
    float time;
    float _padding[3];
};

void Engine::initialize(int backbufferWidth, int backbufferHeight, bool capture)
{
    printf("LeafEngine started\n");

    ResourceManager::create();

    this->backbufferWidth = backbufferWidth;
    this->backbufferHeight = backbufferHeight;
    this->capture = capture;

    this->hwnd = CreateWindow("static", "Leaf", WS_POPUP | (this->capture ? 0 : WS_VISIBLE), 0, 0, this->backbufferWidth, this->backbufferHeight, NULL, NULL, NULL, 0);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    //set buffer dimensions and format
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = this->backbufferWidth;
    swapChainDesc.BufferDesc.Height = this->backbufferHeight;
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &Device::device, NULL, &Device::context);
    CHECK_HRESULT(res);

    res = swapChain->GetBuffer(0, __uuidof(backBuffer), (void **)&backBuffer);
    CHECK_HRESULT(res);

    res = Device::device->CreateRenderTargetView(backBuffer, NULL, &renderTarget);
    CHECK_HRESULT(res);

    Device::context->OMSetRenderTargets(1, &renderTarget, NULL);

    if (this->capture)
    {
        D3D11_TEXTURE2D_DESC captureBufferDesc;
        this->backBuffer->GetDesc(&captureBufferDesc);
        captureBufferDesc.BindFlags = 0;
        captureBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        captureBufferDesc.Usage = D3D11_USAGE_STAGING;

        res = Device::device->CreateTexture2D(&captureBufferDesc, NULL, &this->captureBuffer);
        CHECK_HRESULT(res);
    }

    res = Device::device->CreateVertexShader(plopVS, sizeof(plopVS), NULL, &vs);
    CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    res = Device::device->CreateInputLayout(layout, 3, plopVS, sizeof(plopVS), &inputLayout);
    CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(plopPS, sizeof(plopPS), NULL, &ps);
    CHECK_HRESULT(res);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(SceneState);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    res = Device::device->CreateBuffer(&cbDesc, NULL, &cb);
    CHECK_HRESULT(res);

    startTime = timeGetTime();

    this->mesh = ResourceManager::getInstance()->requestResource<Mesh>("plop");
}

void Engine::shutdown()
{
    printf("LeafEngine stopped\n");

    ResourceManager::getInstance()->releaseResource(this->mesh);

    DestroyWindow(this->hwnd);

    ResourceManager::destroy();
}

void Engine::loadData(cJSON *json)
{
    cJSON *materials = cJSON_GetObjectItem(json, "materials");
    cJSON *material = materials->child;
    while (material)
    {
        std::string name = material->string;
        ResourceManager::getInstance()->updateResourceData<Material>(name, material);
        
        material = material->next;
    }
}

void Engine::render(int width, int height)
{
    float time = (float)(timeGetTime() - startTime) * 0.001f * 140.0f / 60.0f;

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    Device::context->RSSetViewports(1, &viewport);

    float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    Device::context->ClearRenderTargetView(renderTarget, clearColor);

    Device::context->VSSetShader(vs, NULL, 0);
    Device::context->PSSetShader(ps, NULL, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    SceneState *sceneState = (SceneState *)mappedResource.pData;
    sceneState->time = time;
    Device::context->Unmap(cb, 0);
    Device::context->PSSetConstantBuffers(0, 1, &cb);

    Device::context->IASetInputLayout(inputLayout);

    this->mesh->bind();

    Device::context->Draw(this->mesh->getVertexCount(), 0);

    swapChain->Present(0, 0);

    if (this->capture)
        Device::context->CopyResource(this->captureBuffer, this->backBuffer);
}

void Engine::renderBlenderViewport(int width, int height)
{
    assert(this->capture);

    this->render(width, height);

    glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    D3D11_MAPPED_SUBRESOURCE mappedCaptureBuffer;
    HRESULT res = Device::context->Map(this->captureBuffer, 0, D3D11_MAP_READ, 0, &mappedCaptureBuffer);
    CHECK_HRESULT(res);

    // direct copy from D3D mapped memory to GL backbuffer :)
    glRasterPos2i(0, height);
    glPixelZoom(1, -1);
    glDrawPixels(this->backbufferWidth, this->backbufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, mappedCaptureBuffer.pData);

    Device::context->Unmap(this->captureBuffer, 0);
}
