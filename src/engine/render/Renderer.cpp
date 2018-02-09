#include <engine/render/Renderer.h>

#include <cstdio>

#include <windows.h>
#include <gl/GL.h>
#include <d3d11.h>

#include <engine/glm/glm.hpp>
#include <engine/glm/gtc/matrix_inverse.hpp>

#include <engine/render/Device.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Image.h>
#include <engine/render/Material.h>
#include <engine/render/Mesh.h>
#include <engine/render/PostProcessor.h>
#include <engine/render/RenderList.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/ShadowRenderer.h>
#include <engine/render/Texture.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/render/shaders/constants/SceneConstants.h>
#include <engine/resource/ResourceManager.h>
#include <engine/scene/Scene.h>

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

#pragma pack(push)
#pragma pack(16)
struct InstanceData
{
    glm::mat4 modelMatrix;
    glm::mat4 worldToPreviousFrameClipSpaceMatrix;
    glm::mat3x4 normalMatrix; // use 3x4 to match cbuffer packing rules
};
#pragma pack(pop)

Renderer::Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename)
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
    depthBufferDesc.SampleDesc.Count = 1;
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

    this->postProcessor = new PostProcessor(this->renderTarget, backbufferWidth, backbufferHeight);
    this->shadowRenderer = new ShadowRenderer(1024);

    this->motionTarget = new RenderTarget(backbufferWidth, backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    
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
    res = Device::device->CreateInputLayout(layout, 15, standardVS, sizeof(standardVS), &inputLayout);
    CHECK_HRESULT(res);

    // built-in rendering resources

    ResourceManager::getInstance()->updateResourceData<Image>("__default_black", blackDDS, sizeof(blackDDS));
    ResourceManager::getInstance()->updateResourceData<Image>("__default_white", whiteDDS, sizeof(whiteDDS));
    ResourceManager::getInstance()->updateResourceData<Image>("__default_normal", normalDDS, sizeof(normalDDS));

    ResourceManager::getInstance()->updateResourceData<Texture>("__default_black", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_black\"}", 46);
    ResourceManager::getInstance()->updateResourceData<Texture>("__default_white", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_white\"}", 46);
    ResourceManager::getInstance()->updateResourceData<Texture>("__default_normal", (const unsigned char *)"{\"type\": \"IMAGE\", \"image\": \"__default_normal\"}", 47);

    const unsigned char fullscreenQuadData[] = {
        0x04, 0x00, 0x00, 0x00, // vertex count

        // vertex data
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
        0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f, 0x00, 0x00, 0x80, 0x3f,

        0x01, 0x00, 0x00, 0x00, // material count

        // material name
        0x09, 0x00, 0x00, 0x00,
        0x5f, 0x5f, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, // "__default"

        0x06, 0x00, 0x00, 0x00, // index count

        // index data
        0x00, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x03, 0x00, 0x00, 0x00
    };

    ResourceManager::getInstance()->updateResourceData<Mesh>("__fullscreenQuad", fullscreenQuadData, sizeof(fullscreenQuadData));

    this->fullscreenQuad = ResourceManager::getInstance()->requestResource<Mesh>("__fullscreenQuad");

    this->frameGraph = new FrameGraph(profileFilename);
}

Renderer::~Renderer()
{
    delete this->frameGraph;

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

    delete this->renderList;

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    //for (int i = 0; i < GBUFFER_PLANE_COUNT; i++)
    //    delete this->gBuffer[i];

    this->gBufferDepthState->Release();
    this->lightingDepthState->Release();
    this->backgroundDepthState->Release();

    delete this->postProcessor;
    delete this->shadowRenderer;

    delete this->motionTarget;

    // make sure all graphics resources are released before destroying the context
    ResourceManager::getInstance()->clearPendingUnloads();

    Device::context->Release();
    Device::device->Release();
}

void Renderer::render(const Scene *scene, const RenderSettings &settings, float deltaTime)
{
    // rebake environment when needed
    settings.environment.environmentMap->update(this->frameGraph);

    this->renderList->clear();
    scene->fillRenderList(this->renderList);
    this->renderList->sort();

    // shadow maps
	ShadowConstants shadowConstants;
    this->shadowRenderer->render(this->frameGraph, scene, this->renderList, &shadowConstants);

    SceneConstants sceneConstants;
    sceneConstants.ambientColor = settings.environment.ambientColor;
    sceneConstants.mist = settings.environment.mist;
    sceneConstants.motionSpeedFactor = settings.camera.shutterSpeed / deltaTime;
    sceneConstants.motionBlurTileSize = 40.0f;
	sceneConstants.focusDistance = settings.camera.focusDistance;

    const std::vector<RenderList::Light> &lights = this->renderList->getLights();
    sceneConstants.pointLightCount = 0;
    sceneConstants.spotLightCount = 0;
    for (int i = 0; i < lights.size(); i++)
    {
        if (!lights[i].spot && sceneConstants.pointLightCount < MAX_LIGHT)
        {
            int index = sceneConstants.pointLightCount++;
            sceneConstants.pointLights[index].position = lights[i].position;
            sceneConstants.pointLights[index].radius = lights[i].radius;
            sceneConstants.pointLights[index].color = lights[i].color;
        }

        if (lights[i].spot && sceneConstants.spotLightCount < MAX_LIGHT)
        {
            // angle falloff precomputations
            float cosOuterAngle = cosf(lights[i].angle * 0.5f);
            float cosInnerAngle = glm::mix(cosOuterAngle + 0.001f, 1.0f, lights[i].blend);
            float cosAngleScale = 1.0f / (cosInnerAngle - cosOuterAngle);
            float cosAngleOffset = -cosOuterAngle * cosAngleScale;

            int index = sceneConstants.spotLightCount++;
            sceneConstants.spotLights[index].position = lights[i].position;
            sceneConstants.spotLights[index].radius = lights[i].radius;
            sceneConstants.spotLights[index].color = lights[i].color;
            sceneConstants.spotLights[index].cosAngleScale = cosAngleScale;
            sceneConstants.spotLights[index].direction = lights[i].direction;
            sceneConstants.spotLights[index].cosAngleOffset = cosAngleOffset;
            sceneConstants.spotLights[index].scattering = lights[i].scattering;
        }
    }

    this->frameGraph->addClearTarget(this->renderTarget, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
    this->frameGraph->addClearTarget(this->motionTarget->getTarget(), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    this->frameGraph->addClearTarget(this->depthTarget, 1.0, 0);

    RenderTarget *radianceTarget = this->postProcessor->getRadianceTarget();

    Pass *radiancePass = this->frameGraph->addPass("Radiance");
    radiancePass->setTargets({ radianceTarget->getTarget(), this->motionTarget->getTarget() }, this->depthTarget);
	radiancePass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, settings.camera.viewMatrix, settings.camera.projectionMatrix);

    //this->shadowRenderer->bind();

    const std::vector<RenderList::Job> &jobs = this->renderList->getJobs();

    {
        //GPUProfiler::ScopedProfile profile("Geometry");
        Material *currentMaterial = nullptr;
        const Mesh::SubMesh *currentSubMesh = nullptr;
        Batch *currentBatch = nullptr;
        Job *currentJob = nullptr;
        for (const auto &job : jobs)
        {
            if (currentMaterial != job.material)
            {
                currentMaterial = job.material;

                currentBatch = radiancePass->addBatch(std::string("Material"));
                currentBatch->setDepthStencil(this->gBufferDepthState);
                currentBatch->setVertexShader(standardVertexShader);
                currentBatch->setPixelShader(standardPixelShader);
                currentBatch->setInputLayout(this->inputLayout);

                currentMaterial->setupBatch(currentBatch, settings, this->shadowRenderer->getSRV(), this->shadowRenderer->getSampler(), &shadowConstants);
            }

            if (currentSubMesh != job.subMesh)
            {
                currentSubMesh = job.subMesh;

                currentJob = currentBatch->addJob();
                currentJob->setBuffers(currentSubMesh->vertexBuffer, currentSubMesh->indexBuffer, currentSubMesh->indexCount);
            }

			InstanceData instanceData;
			instanceData.modelMatrix = job.transform;
			instanceData.worldToPreviousFrameClipSpaceMatrix = this->previousFrameViewProjectionMatrix * job.previousFrameTransform * glm::inverse(job.transform);
			instanceData.normalMatrix = glm::mat3x4(glm::inverseTranspose(glm::mat3(job.transform)));
		
			currentJob->addInstance(instanceData);
        }
    }

    //this->shadowRenderer->unbind();

    // background
    Batch *backgroundBatch = radiancePass->addBatch("Background");
    backgroundBatch->setDepthStencil(this->gBufferDepthState);
    backgroundBatch->setResources({ settings.environment.environmentMap->getSRV() });
    backgroundBatch->setSamplers({ settings.environment.environmentMap->getSamplerState() });
    backgroundBatch->setVertexShader(this->backgroundVertexShader);
    backgroundBatch->setPixelShader(this->backgroundPixelShader);
    backgroundBatch->setInputLayout(this->inputLayout);

    const Mesh::SubMesh &quadSubMesh = this->fullscreenQuad->getSubMeshes()[0];

    Job *backgroundJob = backgroundBatch->addJob();
    backgroundJob->setBuffers(quadSubMesh.vertexBuffer, quadSubMesh.indexBuffer, quadSubMesh.indexCount);
	backgroundJob->addInstance();

    this->postProcessor->render(this->frameGraph, settings, this->motionTarget);

    this->frameGraph->execute(sceneConstants);

	if (this->capture)
		Device::context->CopyResource(this->captureBuffer, this->backBuffer);

    this->swapChain->Present(0, 0);

    this->previousFrameViewProjectionMatrix = settings.camera.projectionMatrix * settings.camera.viewMatrix;
}

void Renderer::renderBlenderViewport(const Scene *scene, const RenderSettings &settings)
{
    assert(this->capture);

    this->render(scene, settings, 1.0f / 60.0f);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    D3D11_MAPPED_SUBRESOURCE mappedCaptureBuffer;
    HRESULT res = Device::context->Map(this->captureBuffer, 0, D3D11_MAP_READ, 0, &mappedCaptureBuffer);
    CHECK_HRESULT(res);

    // direct copy from D3D mapped memory to GL backbuffer :)
    glRasterPos2i(0, settings.frameHeight - 1);
    glPixelZoom(1, -1);
    glDrawPixels(this->backbufferWidth, this->backbufferHeight - 1, GL_RGBA, GL_UNSIGNED_BYTE, mappedCaptureBuffer.pData);
    glPixelZoom(1, 1);

    Device::context->Unmap(this->captureBuffer, 0);
}
