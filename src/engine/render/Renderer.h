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

    private:
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

        ID3D11VertexShader *backgroundVertexShader;
        ID3D11VertexShader *basicVertexShader;
        ID3D11VertexShader *gbufferVertexShader;
        ID3D11VertexShader *plopVertexShader;
        ID3D11VertexShader *standardVertexShader;

        ID3D11PixelShader *backgroundPixelShader;
        ID3D11PixelShader *basicPixelShader;
        ID3D11PixelShader *gbufferPixelShader;
        ID3D11PixelShader *plopPixelShader;
        ID3D11PixelShader *standardPixelShader;

        ID3D11InputLayout *inputLayout;

        RenderList *renderList;

        Mesh *fullscreenQuad;

        // G-Buffer layout
        // normal (xyz), metalness (w)
        // albedo (xyz), roughness (w)
        static const int GBUFFER_PLANE_COUNT = 2;
        RenderTarget *gBuffer[GBUFFER_PLANE_COUNT];

        ID3D11DepthStencilState *gBufferDepthState;
        ID3D11DepthStencilState *lightingDepthState;
        ID3D11DepthStencilState *backgroundDepthState;

        PostProcessor *postProcessor;
        ShadowRenderer *shadowRenderer;
        RenderTarget *motionTarget;

        glm::mat4 previousFrameViewProjectionMatrix;
};
