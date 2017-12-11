#pragma once

#include <string>
#include <vector>

#include <windows.h>
#include <d3d11_1.h>

#include <engine/render/shaders/constants/PassConstants.h>

class Batch;

/**
 * A pass contains all the batches for a given render target and viewport.
 */
class Pass
{
    public:
        Pass(const std::string &name);

        void setTargets(const std::vector<ID3D11RenderTargetView *> &colorTargets, ID3D11DepthStencilView *depthStencilTarget)
        {
            this->colorTargets = colorTargets;
            this->depthStencilTarget = depthStencilTarget;
        }

        void setViewport(D3D11_VIEWPORT viewport, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);
		void setViewport(float width, float height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix);

        Batch *addBatch(const std::string &name);

        void execute(ID3D11DeviceContext *context, ID3D11Buffer *passConstantBuffer, ID3DUserDefinedAnnotation *annotation);

    private:
        std::string name;

        std::vector<ID3D11RenderTargetView *> colorTargets;
        ID3D11DepthStencilView *depthStencilTarget = nullptr;

        D3D11_VIEWPORT viewport;

        PassConstants passConstants;

        std::vector<Batch *> batches;
};
