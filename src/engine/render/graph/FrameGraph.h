#pragma once

#include <string>
#include <vector>

#include <windows.h>
#include <d3d11.h>

#include <engine/glm/vec4.hpp>

class Pass;

/**
 * The FrameGraph manages all the state changes and submits
 * all draw calls for a given frame.
 */
class FrameGraph
{
    public:
        FrameGraph(const std::string &profileFilename);
        ~FrameGraph();

        void addClearTarget(ID3D11RenderTargetView *target, glm::vec4 color);
        void addClearTarget(ID3D11DepthStencilView *target, float depth, unsigned char stencil);

        Pass *addPass(const std::string &name);

        void execute();

    private:
        void clearAllTargets(ID3D11DeviceContext *context);
        void executeAllPasses(ID3D11DeviceContext *context);

        ID3D11DeviceContext *context;

        std::string profileFilename;

        struct ClearColorTarget
        {
            ID3D11RenderTargetView *target;
            glm::vec4 color;
        };
        std::vector<ClearColorTarget> clearColorTargets;

        struct ClearDepthTarget
        {
            ID3D11DepthStencilView *target;
            float depth;
            unsigned char stencil;
        };
        std::vector<ClearDepthTarget> clearDepthTargets;

        std::vector<Pass *> passes;
};
