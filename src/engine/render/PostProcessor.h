#pragma once

#include <d3d11.h>

class FrameGraph;
class Mesh;
class MotionBlurRenderer;
class RenderTarget;

class PostProcessor
{
    public:
        PostProcessor(ID3D11RenderTargetView *backbufferTarget, int backbufferWidth, int backbufferHeight);
        ~PostProcessor();

        RenderTarget *getRadianceTarget() const { return this->targets[1]; }

        void render(FrameGraph *frameGraph, int width, int height, RenderTarget *motionTarget);

    private:
        ID3D11RenderTargetView *backbufferTarget;
		int backbufferWidth;
		int backbufferHeight;

        RenderTarget *targets[2]; // two is enough to ping-pong between the targets

        ID3D11VertexShader *postprocessVertexShader;
        ID3D11PixelShader *postprocessPixelShader;

        ID3D11VertexShader *fxaaVertexShader;
        ID3D11PixelShader *fxaaPixelShader;

        Mesh *fullscreenQuad;

        MotionBlurRenderer *motionBlurRenderer;
};
