#include <engine/render/Renderer.h>

#include <cstdio>
#include <set>

#include <windows.h>
#include <gl/GL.h>
#include <d3d11.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <RenderDoc/renderdoc_app.h>

#include <engine/render/Device.h>
#include <engine/render/DistanceFieldRenderer.h>
#include <engine/render/graph/GPUProfiler.h>
#include <engine/render/Image.h>
#include <engine/render/Material.h>
#include <engine/render/Mesh.h>
#include <engine/render/PostProcessor.h>
#include <engine/render/RenderList.h>
#include <engine/render/RenderTarget.h>
#include <engine/render/Shaders.h>
#include <engine/render/ShaderCache.h>
#include <engine/render/ShaderVariant.h>
#include <engine/render/ShadowRenderer.h>
#include <engine/render/Text.h>
#include <engine/render/Texture.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/render/shaders/constants/SceneConstants.h>
#include <engine/resource/ResourceManager.h>
#include <engine/scene/Scene.h>

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

Renderer::Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename, const std::string &shaderPath)
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

    this->initializeRenderDoc(hwnd, Device::device);

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
    depthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.SampleDesc.Quality = 0;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    res = Device::device->CreateTexture2D(&depthBufferDesc, NULL, &this->depthBuffer);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_VIEW_DESC depthTargetDesc;
    ZeroMemory(&depthTargetDesc, sizeof(depthTargetDesc));
    depthTargetDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthTargetDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthTargetDesc.Texture2D.MipSlice = 0;

    res = Device::device->CreateDepthStencilView(depthBuffer, &depthTargetDesc, &this->depthTarget);
    CHECK_HRESULT(res);

    D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
    ZeroMemory(&depthSRVDesc, sizeof(depthSRVDesc));
    depthSRVDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    depthSRVDesc.Texture2D.MipLevels = 1;

    res = Device::device->CreateShaderResourceView(depthBuffer, &depthSRVDesc, &this->depthSRV);
    CHECK_HRESULT(res);

    D3D11_DEPTH_STENCIL_DESC depthStateDesc;

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));
    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->lessEqualDepthState);

    ZeroMemory(&depthStateDesc, sizeof(depthStateDesc));
    depthStateDesc.DepthEnable = TRUE;
    depthStateDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    depthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    Device::device->CreateDepthStencilState(&depthStateDesc, &this->equalDepthState);

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

    // fill the screen in black to get a clean startup (even if some baking is done at loading time)
    glm::vec4 clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    Device::context->ClearRenderTargetView(this->renderTarget, (float *)&clearColor);
    this->swapChain->Present(0, 0);
    Device::context->ClearRenderTargetView(this->renderTarget, (float *)&clearColor);
    this->swapChain->Present(0, 0);

    this->postProcessor = new PostProcessor(this->renderTarget, backbufferWidth, backbufferHeight);
    this->shadowRenderer = new ShadowRenderer(1024);
    this->distanceFieldRenderer = new DistanceFieldRenderer;

    this->motionTarget = new RenderTarget(backbufferWidth, backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    this->normalTarget = new RenderTarget(backbufferWidth, backbufferHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

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

    Shaders::loadShaders();
    ShaderCache::create(shaderPath);

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
    this->depthSRV->Release();

    delete this->renderList;

    ResourceManager::getInstance()->releaseResource(this->fullscreenQuad);

    //for (int i = 0; i < GBUFFER_PLANE_COUNT; i++)
    //    delete this->gBuffer[i];

    this->lessEqualDepthState->Release();
    this->equalDepthState->Release();

    delete this->postProcessor;
    delete this->shadowRenderer;
    delete this->distanceFieldRenderer;

    delete this->motionTarget;
    delete this->normalTarget;

    // make sure all graphics resources are released before destroying the context
    ResourceManager::getInstance()->clearPendingUnloads();

    ShaderCache::destroy();
    Shaders::unloadShaders();

    Device::context->Release();
    Device::device->Release();
}

int Renderer::exportShaders(const std::string &exportPath, const std::vector<Material *> materials, const std::vector<Text *> texts) const
{
    // extract unique hashes
    std::set<ShaderCache::Hash> materialHashes;
    std::set<ShaderCache::Hash> sdfHashes;

    for (const Material *material : materials)
        materialHashes.insert(material->getPrefixHash());

    for (const Text *text : texts)
        sdfHashes.insert(text->getPrefixHash());

    // compute all the possible variants
    std::vector<ShaderCache::VariantKey> keys;
    for (auto materialHash : materialHashes)
    {
        keys.push_back({ "forward", materialHash });
    }
    for (auto sdfHash : sdfHashes)
    {
        keys.push_back({ "raymarch-depth", sdfHash });
    }

    // append common shaders with no prefix
    keys.push_back({ "bloomdebug" });
    keys.push_back({ "bloomthreshold" });
    keys.push_back({ "bloomdownsample" });
    keys.push_back({ "bloomaccumulation" });
    keys.push_back({ "postprocess" });
    keys.push_back({ "fxaa" });
    keys.push_back({ "depthonly" });
    keys.push_back({ "background" });
    keys.push_back({ "postprocess" });

    return ShaderCache::getInstance()->exportVariants(exportPath, keys);
}

void Renderer::render(const Scene *scene, const RenderSettings &settings, float deltaTime)
{
    ShaderCache::getInstance()->update();

    // rebake environment when needed
    settings.environment.environmentMap->update(this->frameGraph);

    this->renderList->clear();
    scene->fillRenderList(this->renderList);

    this->distanceFieldRenderer->setRenderList(this->renderList);

    // shadow maps
    this->shadowRenderer->render(this->frameGraph, scene, this->renderList);

    SceneConstants sceneConstants;
    sceneConstants.ambientColor = settings.environment.ambientColor;
    sceneConstants.mist = settings.environment.mist;
    sceneConstants.motionSpeedFactor = settings.camera.shutterSpeed / deltaTime;
    sceneConstants.motionBlurTileSize = 40.0f;
	sceneConstants.focusDistance = settings.camera.focusDistance;
    sceneConstants.environmentMipLevels = (float)settings.environment.environmentMap->getMipLevels() - 1;

    const std::vector<RenderList::Light> &lights = this->renderList->getLights();
    sceneConstants.pointLightCount = 0;
    sceneConstants.spotLightCount = 0;
    for (unsigned int i = 0; i < lights.size(); i++)
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
    this->frameGraph->addClearTarget(this->normalTarget->getTarget(), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
    this->frameGraph->addClearTarget(this->depthTarget, 1.0, 0);

    const std::vector<RenderList::Job> &jobs = this->renderList->getJobs();

    // depth pre-pass
    glm::vec3 cameraDirection = glm::vec3(settings.camera.viewMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 1.0f));
    this->renderList->sortFrontToBack(cameraDirection);

    Pass *depthPrePass = this->frameGraph->addPass("DepthPrePass");
    depthPrePass->setTargets({ this->normalTarget->getTarget() }, this->depthTarget);
    depthPrePass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, settings.camera.viewMatrix, settings.camera.projectionMatrix);

    const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("depthonly");
    Pipeline pipeline = shaderVariant->getPipeline();
    pipeline.inputLayout = Shaders::layout.depthOnly;
    pipeline.depthStencil = this->lessEqualDepthState;

    Batch *depthBatch = depthPrePass->addBatch("");
    depthBatch->setPipeline(pipeline);

    const Mesh::SubMesh *currentSubMesh = nullptr;
    Job *currentJob = nullptr;
    for (const auto &job : jobs)
    {
        if (currentSubMesh != job.subMesh)
        {
            currentSubMesh = job.subMesh;

            currentJob = depthBatch->addJob();
            currentJob->setBuffers(currentSubMesh->vertexBuffer, currentSubMesh->indexBuffer, currentSubMesh->indexCount);
        }

        DepthOnlyInstanceData instanceData;
        instanceData.transformMatrix = job.transform;

        currentJob->addInstance(instanceData);
    }

    this->distanceFieldRenderer->addPrePassJobs(depthPrePass);
    
    // main radiance pass
    this->renderList->sortByMaterial();

    RenderTarget *radianceTarget = this->postProcessor->getRadianceTarget();

    Pass *radiancePass = this->frameGraph->addPass("Radiance");
    radiancePass->setTargets({ radianceTarget->getTarget(), this->motionTarget->getTarget() }, this->depthTarget);
	radiancePass->setViewport((float)this->backbufferWidth, (float)this->backbufferHeight, settings.camera.viewMatrix, settings.camera.projectionMatrix);

    DescriptorSet environmentParameterBlock = {
        { settings.environment.environmentMap->getSRV() },
        {},
        { settings.environment.environmentMap->getSamplerState() },
        {}
    };

    this->distanceFieldRenderer->addDeferredJobs(radiancePass, this->shadowRenderer->getParameterBlock(), environmentParameterBlock);

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

                const ShaderVariant *shaderVariant = ShaderCache::getInstance()->getVariant("forward", currentMaterial->getPrefixHash());
                Pipeline pipeline = shaderVariant->getPipeline();
                pipeline.inputLayout = Shaders::layout.instancedMesh;
                pipeline.depthStencil = this->equalDepthState;
                currentBatch->setPipeline(pipeline);

                currentBatch->setDescriptorSets({
                    currentMaterial->getParameterBlock(),
                    this->shadowRenderer->getParameterBlock(),
                    environmentParameterBlock
                });
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

    this->distanceFieldRenderer->clearRenderList();

    // background
    Batch *backgroundBatch = radiancePass->addBatch("Background");

    const ShaderVariant *backgroundShader = ShaderCache::getInstance()->getVariant("background");
    Pipeline backgroundPipeline = backgroundShader->getPipeline();
    backgroundPipeline.inputLayout = Shaders::layout.instancedMesh;
    backgroundPipeline.depthStencil = this->equalDepthState;
    backgroundBatch->setPipeline(backgroundPipeline);

    backgroundBatch->setDescriptorSets({ environmentParameterBlock });

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

void Renderer::renderBlenderFrame(const Scene *scene, const RenderSettings &settings, float *outputBuffer, float deltaTime)
{
    assert(this->capture);

    this->render(scene, settings, deltaTime);

    D3D11_MAPPED_SUBRESOURCE mappedCaptureBuffer;
    HRESULT res = Device::context->Map(this->captureBuffer, 0, D3D11_MAP_READ, 0, &mappedCaptureBuffer);
    CHECK_HRESULT(res);

    unsigned char *byteData = (unsigned char *)mappedCaptureBuffer.pData;

    // flip the image vertically
    byteData += mappedCaptureBuffer.RowPitch * (settings.frameHeight - 1);
    for (int y = 0; y < settings.frameHeight; y++)
    {
        for (int i = 0; i < settings.frameWidth; i++)
        {
            // convert unsigned bytes to float values
            *outputBuffer++ = (float)(*byteData++) / 255.0f;
            *outputBuffer++ = (float)(*byteData++) / 255.0f;
            *outputBuffer++ = (float)(*byteData++) / 255.0f;
            *outputBuffer++ = 1.0f;
            byteData++;
        }
        byteData -= settings.frameWidth * 4 /* RGBA */;
        byteData -= mappedCaptureBuffer.RowPitch;
    }

    Device::context->Unmap(this->captureBuffer, 0);
}

void Renderer::initializeRenderDoc(HWND hwnd, ID3D11Device *device)
{
    HMODULE renderDocModule = GetModuleHandle("renderdoc.dll");
    if (renderDocModule != NULL)
    {
        printf("RenderDoc found!\n");

        pRENDERDOC_GetAPI renderDocGetApi = (pRENDERDOC_GetAPI)GetProcAddress(renderDocModule, "RENDERDOC_GetAPI");

        RENDERDOC_API_1_1_1 *renderDoc;
        if (renderDocGetApi(eRENDERDOC_API_Version_1_1_1, (void **)&renderDoc))
        {
            renderDoc->SetActiveWindow(device, (void *)hwnd);
        }
        else
        {
            printf("Failed to load the RenderDoc API\n");
        }
    }
}
