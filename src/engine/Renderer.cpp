#include <engine/Renderer.h>

#include <cstdio>

#include <windows.h>
#include <gl/GL.h>
#include <d3d11.h>

#include <engine/glm/glm.hpp>
#include <engine/glm/gtc/matrix_inverse.hpp>

#include <engine/Device.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/RenderList.h>
#include <engine/ResourceManager.h>
#include <engine/Scene.h>

#include <shaders/basic.vs.hlsl.h>
#include <shaders/basic.ps.hlsl.h>

#pragma pack(push)
#pragma pack(16)
struct SceneState
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec3 cameraPosition;
    float time;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct InstanceData
{
    glm::mat4 modelMatrix;
    glm::mat3 normalMatrix;
    float _padding[3];
};
#pragma pack(pop)

Renderer::Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture)
{
    this->backbufferWidth = backbufferWidth;
    this->backbufferHeight = backbufferHeight;
    this->capture = capture;
    this->renderList = new RenderList;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

    //set buffer dimensions and format
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = backbufferWidth;
    swapChainDesc.BufferDesc.Height = backbufferHeight;
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

    res = swapChain->GetBuffer(0, __uuidof(this->backBuffer), (void **)&this->backBuffer);
    CHECK_HRESULT(res);

    res = Device::device->CreateRenderTargetView(this->backBuffer, NULL, &this->renderTarget);
    CHECK_HRESULT(res);

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = this->backbufferWidth;
    depthBufferDesc.Height = this->backbufferHeight;
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    res = Device::device->CreateTexture2D(&depthBufferDesc, NULL, &this->depthBuffer);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthTargetDesc;
    ZeroMemory(&depthTargetDesc, sizeof(depthTargetDesc));
    depthTargetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTargetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthTargetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(depthBuffer, &depthTargetDesc, &this->depthTarget);
    CHECK_HRESULT(res);

    Device::context->OMSetRenderTargets(1, &this->renderTarget, this->depthTarget);

    D3D11_DEPTH_STENCIL_DESC depthStateDesc;
    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));
    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

    ID3D11DepthStencilState *depthState;
    Device::device->CreateDepthStencilState(&depthStateDesc, &depthState);
    Device::context->OMSetDepthStencilState(depthState, 0);

    D3D11_RASTERIZER_DESC rasterizerDesc;
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = TRUE;
    rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    ID3D11RasterizerState *rasterizerState;
    Device::device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);
    Device::context->RSSetState(rasterizerState);

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

    res = Device::device->CreateVertexShader(basicVS, sizeof(basicVS), NULL, &vs);
    CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    res = Device::device->CreateInputLayout(layout, 3, basicVS, sizeof(basicVS), &inputLayout);
    CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(basicPS, sizeof(basicPS), NULL, &ps);
    CHECK_HRESULT(res);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof(SceneState);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->cbScene);
    CHECK_HRESULT(res);

    cbDesc.ByteWidth = sizeof(Material::MaterialData);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->cbMaterial);
    CHECK_HRESULT(res);

    cbDesc.ByteWidth = sizeof(InstanceData);
    res = Device::device->CreateBuffer(&cbDesc, NULL, &this->cbInstance);
    CHECK_HRESULT(res);

    ID3D11Buffer *allConstantBuffers[] = { this->cbScene, this->cbMaterial, this->cbInstance };
    Device::context->VSSetConstantBuffers(0, 3, allConstantBuffers);
    Device::context->PSSetConstantBuffers(0, 3, allConstantBuffers);
}

Renderer::~Renderer()
{
    delete this->renderList;
}

void Renderer::render(const Scene *scene, int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, float time)
{
    D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    Device::context->RSSetViewports(1, &viewport);

    float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    Device::context->ClearRenderTargetView(this->renderTarget, clearColor);
    Device::context->ClearDepthStencilView(this->depthTarget, D3D11_CLEAR_DEPTH, 1.0f, 0);

    Device::context->VSSetShader(vs, NULL, 0);
    Device::context->PSSetShader(ps, NULL, 0);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    SceneState *sceneState = (SceneState *)mappedResource.pData;
    sceneState->viewMatrix = viewMatrix;
    sceneState->projectionMatrix = projectionMatrix;
    glm::mat4 viewMatrixInverse = glm::inverse(viewMatrix);
    sceneState->cameraPosition = glm::vec3(viewMatrixInverse[3][0], viewMatrixInverse[3][1], viewMatrixInverse[3][2]);
    sceneState->time = time;
    Device::context->Unmap(this->cbScene, 0);

    Device::context->IASetInputLayout(inputLayout);

    this->renderList->clear();
    scene->fillRenderList(this->renderList);
    this->renderList->sort();

    const std::vector<RenderList::Job> &jobs = this->renderList->getJobs();

    Material *currentMaterial = nullptr;
    Mesh * currentMesh = nullptr;
    std::for_each(jobs.begin(), jobs.end(), [&](const RenderList::Job &job)
    {
        if (currentMaterial != job.material)
        {
            currentMaterial = job.material;

            HRESULT res = Device::context->Map(this->cbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
            CHECK_HRESULT(res);
            memcpy(mappedResource.pData, &currentMaterial->getMaterialData(), sizeof(Material::MaterialData));
            Device::context->Unmap(this->cbMaterial, 0);
        }

        if (currentMesh != job.mesh)
        {
            currentMesh = job.mesh;
            currentMesh->bind();
        }

        HRESULT res = Device::context->Map(this->cbInstance, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        CHECK_HRESULT(res);
        InstanceData *instanceData = (InstanceData *)mappedResource.pData;
        instanceData->modelMatrix = job.transform;
        instanceData->normalMatrix = glm::mat3(glm::inverseTranspose(job.transform));
        Device::context->Unmap(this->cbInstance, 0);

        Device::context->Draw(currentMesh->getVertexCount(), 0);
    });

    swapChain->Present(0, 0);

    if (this->capture)
        Device::context->CopyResource(this->captureBuffer, this->backBuffer);
}

void Renderer::renderBlenderViewport(const Scene *scene, int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, float time)
{
    assert(this->capture);

    this->render(scene, width, height, viewMatrix, projectionMatrix, time);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    D3D11_MAPPED_SUBRESOURCE mappedCaptureBuffer;
    HRESULT res = Device::context->Map(this->captureBuffer, 0, D3D11_MAP_READ, 0, &mappedCaptureBuffer);
    CHECK_HRESULT(res);

    // direct copy from D3D mapped memory to GL backbuffer :)
    glRasterPos2i(0, height - 1);
    glPixelZoom(1, -1);
    glDrawPixels(this->backbufferWidth, this->backbufferHeight - 1, GL_RGBA, GL_UNSIGNED_BYTE, mappedCaptureBuffer.pData);

    Device::context->Unmap(this->captureBuffer, 0);
}
