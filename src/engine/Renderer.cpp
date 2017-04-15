#include <engine/Renderer.h>

#include <cstdio>

#include <windows.h>
#include <gl/GL.h>
#include <d3d11.h>

#include <engine/glm/glm.hpp>
#include <engine/glm/gtc/matrix_inverse.hpp>

#include <engine/Device.h>
#include <engine/GPUProfiler.h>
#include <engine/Image.h>
#include <engine/Material.h>
#include <engine/Mesh.h>
#include <engine/PostProcessor.h>
#include <engine/RenderList.h>
#include <engine/RenderTarget.h>
#include <engine/ResourceManager.h>
#include <engine/Scene.h>
#include <engine/ShadowRenderer.h>
#include <engine/Texture.h>

#include <shaders/background.vs.hlsl.h>
#include <shaders/background.ps.hlsl.h>
#include <shaders/basic.vs.hlsl.h>
#include <shaders/basic.ps.hlsl.h>
#include <shaders/gbuffer.vs.hlsl.h>
#include <shaders/gbuffer.ps.hlsl.h>
#include <shaders/plop.vs.hlsl.h>
#include <shaders/plop.ps.hlsl.h>
#include <shaders/standard.vs.hlsl.h>
#include <shaders/standard.ps.hlsl.h>

static const unsigned char blackDDS[] = { 68, 68, 83, 32, 124, 0, 0, 0, 7, 16, 2, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 86, 69, 82, 0, 0, 0, 0, 78, 86, 84, 84, 0, 1, 2, 0, 32, 0, 0, 0, 4, 0, 0, 0, 68, 88, 49, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 16, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 170, 170, 170, 170, 0, 0, 0, 0, 170, 170, 170, 170, 0, 0, 0, 0, 170, 170, 170, 170 };
static const unsigned char whiteDDS[] = { 68, 68, 83, 32, 124, 0, 0, 0, 7, 16, 2, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 86, 69, 82, 0, 0, 0, 0, 78, 86, 84, 84, 0, 1, 2, 0, 32, 0, 0, 0, 4, 0, 0, 0, 68, 88, 49, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 16, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 170, 170, 170, 170, 255, 255, 255, 255, 170, 170, 170, 170, 255, 255, 255, 255, 170, 170, 170, 170 };
static const unsigned char normalDDS[] = { 68, 68, 83, 32, 124, 0, 0, 0, 7, 16, 2, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 85, 86, 69, 82, 0, 0, 0, 0, 78, 86, 84, 84, 0, 1, 2, 0, 32, 0, 0, 0, 4, 0, 0, 128, 68, 88, 49, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 16, 64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 71, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 255, 139, 31, 124, 255, 255, 255, 255, 255, 139, 31, 124, 255, 255, 255, 255, 255, 139, 31, 124, 255, 255, 255, 255 };

#define MAX_LIGHT 8

#pragma pack(push)
#pragma pack(16)
struct PointLightData
{
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float _padding;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct SpotLightData
{
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float cosAngleScale;
    glm::vec3 direction;
    float cosAngleOffset;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct SceneState
{
    glm::mat4 viewMatrix;
    glm::mat4 viewMatrixInverse;
    glm::mat4 projectionMatrix;
    glm::mat4 projectionMatrixInverse;
    glm::mat4 viewProjectionInverseMatrix;
    glm::vec3 cameraPosition;
    int pointLightCount;
    int spotLightCount;
    glm::vec3 ambientColor;
    PointLightData pointLights[MAX_LIGHT];
    SpotLightData spotLights[MAX_LIGHT];
    float mist;
    float _padding[3];
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct InstanceData
{
    glm::mat4 modelMatrix;
    glm::mat3x4 normalMatrix; // use 3x4 to match cbuffer packing rules
};
#pragma pack(pop)

Renderer::Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename)
{
    this->backbufferWidth = backbufferWidth;
    this->backbufferHeight = backbufferHeight;
    this->capture = capture;
    this->profileFilename = profileFilename;
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
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &this->swapChain, &Device::device, NULL, &Device::context);
    CHECK_HRESULT(res);

    res = this->swapChain->GetBuffer(0, __uuidof(this->backBuffer), (void **)&this->backBuffer);
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
    depthBufferDesc.SampleDesc.Count = 4;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    res = Device::device->CreateTexture2D(&depthBufferDesc, NULL, &this->depthBuffer);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthTargetDesc;
    ZeroMemory(&depthTargetDesc, sizeof(depthTargetDesc));
    depthTargetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTargetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    depthTargetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(depthBuffer, &depthTargetDesc, &this->depthTarget);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_DESC depthStateDesc;
    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));

    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->gBufferDepthState);

    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_GREATER;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->lightingDepthState);

    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->backgroundDepthState);

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

    //for (int i = 0; i < GBUFFER_PLANE_COUNT; i++)
    //    this->gBuffer[i] = new RenderTarget(this->backbufferWidth, this->backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

    this->postProcessor = new PostProcessor(this->renderTarget);
    this->shadowRenderer = new ShadowRenderer(1024);

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

    res = Device::device->CreateVertexShader(backgroundVS, sizeof(backgroundVS), NULL, &backgroundVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(basicVS, sizeof(basicVS), NULL, &basicVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(gbufferVS, sizeof(gbufferVS), NULL, &gbufferVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(plopVS, sizeof(plopVS), NULL, &plopVertexShader); CHECK_HRESULT(res);
    res = Device::device->CreateVertexShader(standardVS, sizeof(standardVS), NULL, &standardVertexShader); CHECK_HRESULT(res);

    res = Device::device->CreatePixelShader(backgroundPS, sizeof(backgroundPS), NULL, &backgroundPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(basicPS, sizeof(basicPS), NULL, &basicPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(gbufferPS, sizeof(gbufferPS), NULL, &gbufferPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(plopPS, sizeof(plopPS), NULL, &plopPixelShader); CHECK_HRESULT(res);
    res = Device::device->CreatePixelShader(standardPS, sizeof(standardPS), NULL, &standardPixelShader); CHECK_HRESULT(res);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    res = Device::device->CreateInputLayout(layout, 4, basicVS, sizeof(basicVS), &inputLayout);
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

    // built-in rendering resources

    ResourceManager::getInstance()->updateResourceData<Image>("__default_black", blackDDS, sizeof(blackDDS));
    ResourceManager::getInstance()->updateResourceData<Image>("__default_white", whiteDDS, sizeof(whiteDDS));
    ResourceManager::getInstance()->updateResourceData<Image>("__default_normal", normalDDS, sizeof(normalDDS));

    ResourceManager::getInstance()->updateResourceData<Texture>("__default_black", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_black\"}", 46);
    ResourceManager::getInstance()->updateResourceData<Texture>("__default_white", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_white\"}", 46);
    ResourceManager::getInstance()->updateResourceData<Texture>("__default_normal", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_normal\"}", 47);

    const char *fullscreenQuad = "{"
            "\"vertices\": ["
                "-1, -1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1,"
                "-1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,"
                "1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0,"
                "1, -1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1"
            "],"
            "\"vertexCount\": 4,"
            "\"indices\": ["
                "0, 2, 1,"
                "2, 0, 3"
            "],"
            "\"indexCount\": 6,"
            "\"material\": \"__default\","
            "\"minBound\": [-1.0, -1.0, 0.0],"
            "\"maxBound\": [1.0, 1.0, 0.0]"
        "}";
    ResourceManager::getInstance()->updateResourceData<Mesh>("__fullscreenQuad", (const unsigned char *)fullscreenQuad, strlen(fullscreenQuad));

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");

    GPUProfiler::create(!this->profileFilename.empty());
    GPUProfiler::getInstance()->beginJsonCapture();
}

Renderer::~Renderer()
{
    GPUProfiler::getInstance()->endJsonCapture(this->profileFilename);
    GPUProfiler::destroy();

    this->swapChain->Release();
    this->backBuffer->Release();
    this->depthBuffer->Release();
    if (this->capture)
        this->captureBuffer->Release();
    this->renderTarget->Release();
    this->depthTarget->Release();

    this->backgroundVertexShader->Release();
    this->basicVertexShader->Release();
    this->gbufferVertexShader->Release();
    this->plopVertexShader->Release();
    this->standardVertexShader->Release();

    this->backgroundPixelShader->Release();
    this->basicPixelShader->Release();
    this->gbufferPixelShader->Release();
    this->plopPixelShader->Release();
    this->standardPixelShader->Release();

    this->inputLayout->Release();

    this->cbScene->Release();
    this->cbMaterial->Release();
    this->cbInstance->Release();

    delete this->renderList;

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    //for (int i = 0; i < GBUFFER_PLANE_COUNT; i++)
    //    delete this->gBuffer[i];

    this->gBufferDepthState->Release();
    this->lightingDepthState->Release();
    this->backgroundDepthState->Release();

    delete this->postProcessor;
    delete this->shadowRenderer;

    // make sure all graphics resources are released before destroying the context
    ResourceManager::getInstance()->clearPendingUnloads();

    Device::context->Release();
    Device::device->Release();
}

void Renderer::render(const Scene *scene, int width, int height, bool overrideCamera, const glm::mat4 &viewMatrixOverride, const glm::mat4 &projectionMatrixOverride)
{
    GPUProfiler::getInstance()->beginFrame();

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    if (overrideCamera)
    {
        // use the provided camera parameters
        viewMatrix = viewMatrixOverride;
        projectionMatrix = projectionMatrixOverride;
    }
    else
    {
        // get camera from scene
        scene->setupCameraMatrices(viewMatrix, projectionMatrix, (float)width / (float)height);
    }

    this->renderList->clear();
    scene->fillRenderList(this->renderList);
    this->renderList->sort();

    Device::context->IASetInputLayout(inputLayout);

    // shadow maps
    this->shadowRenderer->render(scene, this->renderList);

    D3D11_VIEWPORT viewport;
    viewport.Width = (float)backbufferWidth;
    viewport.Height = (float)backbufferHeight;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    Device::context->RSSetViewports(1, &viewport);

    float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
    {
        GPUProfiler::ScopedProfile profile("Clear");
        Device::context->ClearRenderTargetView(this->renderTarget, clearColor);
        Device::context->ClearDepthStencilView(this->depthTarget, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->cbScene, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    SceneState *sceneState = (SceneState *)mappedResource.pData;
    sceneState->viewMatrix = viewMatrix;
    glm::mat4 viewMatrixInverse = glm::inverse(viewMatrix);
    sceneState->viewMatrixInverse = viewMatrixInverse;
    sceneState->projectionMatrix = projectionMatrix;
    sceneState->projectionMatrixInverse = glm::inverse(projectionMatrix);
    sceneState->viewProjectionInverseMatrix = glm::inverse(projectionMatrix * viewMatrix);
    sceneState->cameraPosition = glm::vec3(viewMatrixInverse[3][0], viewMatrixInverse[3][1], viewMatrixInverse[3][2]);
    sceneState->ambientColor = scene->getAmbientColor();
    sceneState->mist = scene->getMist();

    const std::vector<RenderList::Light> &lights = this->renderList->getLights();
    sceneState->pointLightCount = 0;
    sceneState->spotLightCount = 0;
    for (int i = 0; i < lights.size(); i++)
    {
        if (!lights[i].spot && sceneState->pointLightCount < MAX_LIGHT)
        {
            int index = sceneState->pointLightCount++;
            sceneState->pointLights[index].position = lights[i].position;
            sceneState->pointLights[index].radius = lights[i].radius;
            sceneState->pointLights[index].color = lights[i].color;
        }

        if (lights[i].spot && sceneState->spotLightCount < MAX_LIGHT)
        {
            // angle falloff precomputations
            float cosOuterAngle = cosf(lights[i].angle * 0.5f);
            float cosInnerAngle = glm::mix(cosOuterAngle + 0.001f, 1.0f, lights[i].blend);
            float cosAngleScale = 1.0f / (cosInnerAngle - cosOuterAngle);
            float cosAngleOffset = -cosOuterAngle * cosAngleScale;

            int index = sceneState->spotLightCount++;
            sceneState->spotLights[index].position = lights[i].position;
            sceneState->spotLights[index].radius = lights[i].radius;
            sceneState->spotLights[index].color = lights[i].color;
            sceneState->spotLights[index].cosAngleScale = cosAngleScale;
            sceneState->spotLights[index].direction = lights[i].direction;
            sceneState->spotLights[index].cosAngleOffset = cosAngleOffset;
        }
    }

    Device::context->Unmap(this->cbScene, 0);

    Device::context->OMSetDepthStencilState(this->gBufferDepthState, 0);

    RenderTarget *radianceTarget = this->postProcessor->getRadianceTarget();
    ID3D11RenderTargetView *radianceTargetView = radianceTarget->getTarget();
    Device::context->OMSetRenderTargets(1, &radianceTargetView, this->depthTarget);

    Device::context->VSSetShader(standardVertexShader, NULL, 0);
    Device::context->PSSetShader(standardPixelShader, NULL, 0);

    ID3D11Buffer *allConstantBuffers[] = { this->cbScene, this->cbMaterial, this->cbInstance };
    Device::context->VSSetConstantBuffers(0, 3, allConstantBuffers);
    Device::context->PSSetConstantBuffers(0, 3, allConstantBuffers);

    this->shadowRenderer->bind();

    const std::vector<RenderList::Job> &jobs = this->renderList->getJobs();

    {
        GPUProfiler::ScopedProfile profile("Geometry");
        Material *currentMaterial = nullptr;
        Mesh * currentMesh = nullptr;
        for (const auto &job : jobs)
        {
            if (currentMaterial != job.material)
            {
                currentMaterial = job.material;

                HRESULT res = Device::context->Map(this->cbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
                CHECK_HRESULT(res);
                memcpy(mappedResource.pData, &currentMaterial->getMaterialData(), sizeof(Material::MaterialData));
                Device::context->Unmap(this->cbMaterial, 0);

                currentMaterial->bindTextures();
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
            instanceData->normalMatrix = glm::mat3x4(glm::inverseTranspose(glm::mat3(job.transform)));
            Device::context->Unmap(this->cbInstance, 0);

            Device::context->DrawIndexed(currentMesh->getIndexCount(), 0, 0);
        }
    }

    this->shadowRenderer->unbind();

    // background pass
    {
        GPUProfiler::ScopedProfile profile("Background");

        /*viewport.Width = (float)width;
        viewport.Height = (float)height;
        Device::context->RSSetViewports(1, &viewport);*/
        Device::context->VSSetShader(backgroundVertexShader, NULL, 0);
        Device::context->PSSetShader(backgroundPixelShader, NULL, 0);
        Device::context->OMSetDepthStencilState(this->backgroundDepthState, 0);
        this->fullscreenQuad->bind();
        Device::context->DrawIndexed(this->fullscreenQuad->getIndexCount(), 0, 0);
    }

    this->postProcessor->render(width, height);

    {
        GPUProfiler::ScopedProfile profile("V-Sync");
        this->swapChain->Present(0, 0);
    }

    GPUProfiler::getInstance()->endFrame();

    if (this->capture)
        Device::context->CopyResource(this->captureBuffer, this->backBuffer);
}

void Renderer::renderBlenderViewport(const Scene *scene, int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
    assert(this->capture);

    this->render(scene, width, height, true, viewMatrix, projectionMatrix);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    D3D11_MAPPED_SUBRESOURCE mappedCaptureBuffer;
    HRESULT res = Device::context->Map(this->captureBuffer, 0, D3D11_MAP_READ, 0, &mappedCaptureBuffer);
    CHECK_HRESULT(res);

    // direct copy from D3D mapped memory to GL backbuffer :)
    glRasterPos2i(0, height - 1);
    glPixelZoom(1, -1);
    glDrawPixels(this->backbufferWidth, this->backbufferHeight - 1, GL_RGBA, GL_UNSIGNED_BYTE, mappedCaptureBuffer.pData);
    glPixelZoom(1, 1);

    Device::context->Unmap(this->captureBuffer, 0);
}
