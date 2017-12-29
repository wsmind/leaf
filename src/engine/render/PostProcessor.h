#pragma once

#include <d3d11.h>

class BloomRenderer;
class FrameGraph;
class Mesh;
class MotionBlurRenderer;
struct RenderSettings;
class RenderTarget;

class PostProcessor
{
    public:
        PostProcessor(ID3D11RenderTargetView *backbufferTarget, int backbufferWidth, int backbufferHeight);
        ~PostProcessor();

        RenderTarget *getRadianceTarget() const { return this->targets[0]; }

        void render(FrameGraph *frameGraph, const RenderSettings &settings, int width, int height, RenderTarget *motionTarget);

    private:
        ID3D11RenderTargetView *backbufferTarget;
		int backbufferWidth;
		int backbufferHeight;

        RenderTarget *targets[2]; // two is enough to ping-pong between the targets

        ID3D11VertexShader *postprocessVertexShader;
        ID3D11PixelShader *postprocessPixelShader;

        ID3D11VertexShader *fxaaVertexShader;
        ID3D11PixelShader *fxaaPixelShader;

		ID3D11InputLayout *inputLayout;

        Mesh *fullscreenQuad;

        MotionBlurRenderer *motionBlurRenderer;
		BloomRenderer *bloomRenderer;
};
