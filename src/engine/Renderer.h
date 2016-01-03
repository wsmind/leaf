#pragma once

#include <windows.h>
#include <d3d11.h>

#include <engine/glm/glm.hpp>

class Mesh;
class RenderList;
class Scene;

class Renderer
{
    public:
        Renderer(HWND hwnd, int backbufferWidth, int backbufferHeight, bool capture);
        ~Renderer();

        void render(const Scene *scene, int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, float time);
        void renderBlenderViewport(const Scene *scene, int width, int height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix, float time);

    private:
        int backbufferWidth;
        int backbufferHeight;
        bool capture;

        IDXGISwapChain *swapChain;
        ID3D11Texture2D *backBuffer;
        ID3D11Texture2D *depthBuffer;
        ID3D11Texture2D *captureBuffer;
        ID3D11RenderTargetView *renderTarget;
        ID3D11DepthStencilView *depthTarget;

        ID3D11VertexShader *vs;
        ID3D11PixelShader *ps;
        ID3D11InputLayout *inputLayout;

        ID3D11Buffer *cbScene;
        ID3D11Buffer *cbMaterial;
        ID3D11Buffer *cbInstance;

        RenderList *renderList;
};
