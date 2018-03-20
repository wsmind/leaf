#pragma once

#include <string>

#include <windows.h>
#include <d3d11.h>

#include <engine/glm/glm.hpp>

class FrameGraph;
class Mesh;
class PostProcessor;
class RenderList;
struct RenderSettings;
class RenderTarget;
class Scene;
class ShadowRenderer;

class Renderer
{
    public:
        Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture, const std::string &profileFilename);
        ~Renderer();

        void render(const Scene *scene, const RenderSettings &settings, float deltaTime);
        void renderBlenderViewport(const Scene *scene, const RenderSettings &settings);
        void renderBlenderFrame(const Scene *scene, const RenderSettings &settings, float *outputBuffer, float deltaTime);

    private:
        void initializeRenderDoc(HWND hwnd, ID3D11Device *device);

        int backbufferWidth;
        int backbufferHeight;
        bool capture;

        FrameGraph *frameGraph;

        IDXGISwapChain *swapChain;
        ID3D11Texture2D *backBuffer;
        ID3D11Texture2D *depthBuffer;
        ID3D11Texture2D *captureBuffer;
        ID3D11RenderTargetView *renderTarget;
        ID3D11DepthStencilView *depthTarget;

        ID3D11InputLayout *inputLayout;
        ID3D11InputLayout *depthOnlyInputLayout;

        RenderList *renderList;

        Mesh *fullscreenQuad;

        // G-Buffer layout
        // normal (xyz), metalness (w)
        // albedo (xyz), roughness (w)
        static const int GBUFFER_PLANE_COUNT = 2;
        RenderTarget *gBuffer[GBUFFER_PLANE_COUNT];

        ID3D11DepthStencilState *lessEqualDepthState;
        ID3D11DepthStencilState *equalDepthState;

        PostProcessor *postProcessor;
        ShadowRenderer *shadowRenderer;
        RenderTarget *motionTarget;

        glm::mat4 previousFrameViewProjectionMatrix;
};
