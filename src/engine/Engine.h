#pragma once

#include <cassert>

#include <windows.h>
#include <d3d11.h>

struct cJSON;

class Mesh;

class Engine
{
    public:
        void initialize(int backbufferWidth, int backbufferHeight, bool capture);
        void shutdown();

        void loadData(cJSON *json);

        void render(int width, int height);
        void renderBlenderViewport(int width, int height);

    private:
        static Engine *instance;

        int backbufferWidth;
        int backbufferHeight;
        bool capture;

        HWND hwnd;

        IDXGISwapChain *swapChain;
        ID3D11Texture2D *backBuffer;
        ID3D11Texture2D *captureBuffer;
        ID3D11RenderTargetView *renderTarget;

        ID3D11VertexShader *vs;
        ID3D11PixelShader *ps;
        ID3D11InputLayout *inputLayout;

        Mesh *mesh;
        ID3D11Buffer *cb;

        DWORD startTime;

public:
        // singleton implementation
        static void create() { assert(!Engine::instance); Engine::instance = new Engine; }
        static void destroy() { assert(Engine::instance); delete Engine::instance; }
        static Engine *getInstance() { assert(Engine::instance); return Engine::instance; }
};
