#pragma once

#include <string>
#include <vector>

#include <windows.h>
#include <d3d11.h>

/**
 * A pass contains all the batches for a given render target and viewport.
 */
class Pass
{
    public:
        Pass(const std::string &name);
        ~Pass();

        void setTargets(const std::vector<ID3D11RenderTargetView *> &colorTargets, ID3D11DepthStencilView *depthStencilTarget);
        void setViewport(D3D11_VIEWPORT viewport);

        void execute(ID3D11DeviceContext *context);

    private:
        std::string name;

        std::vector<ID3D11RenderTargetView *> colorTargets;
        ID3D11DepthStencilView *depthStencilTarget;

        D3D11_VIEWPORT viewport;
};
