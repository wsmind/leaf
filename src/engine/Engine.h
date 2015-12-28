#pragma once

#include <cassert>

#include <windows.h>
#include <d3d11.h>

class Engine
{
    public:
        Engine();
        ~Engine();

        void render();
        void renderBlenderViewport();

    private:
        static Engine *instance;

        int width;
        int height;
        HWND hwnd;

        IDXGISwapChain *swapChain;
        ID3D11RenderTargetView *renderTarget;

        ID3D11Device *device;
        ID3D11DeviceContext *context;

        ID3D11VertexShader *vs;
        ID3D11PixelShader *ps;
        ID3D11InputLayout *inputLayout;

        ID3D11Buffer *vb;
        ID3D11Buffer *cb;

        DWORD startTime;

public:
        // singleton implementation
        static void create() { assert(!Engine::instance); Engine::instance = new Engine; }
        static void destroy() { assert(Engine::instance); delete Engine::instance; }
        static Engine *getInstance() { assert(Engine::instance); return Engine::instance; }
};
