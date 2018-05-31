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

        void render(FrameGraph *frameGraph, const RenderSettings &settings, RenderTarget *motionTarget);

    private:
        ID3D11RenderTargetView *backbufferTarget;
		int backbufferWidth;
		int backbufferHeight;

        RenderTarget *targets[2]; // two is enough to ping-pong between the targets

		ID3D11InputLayout *inputLayout;

        ID3D11Buffer *constantBuffer;

        Mesh *fullscreenQuad;

        MotionBlurRenderer *motionBlurRenderer;
		BloomRenderer *bloomRenderer;
};
