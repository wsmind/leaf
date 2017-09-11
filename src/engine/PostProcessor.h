#pragma once

#include <d3d11.h>

class Mesh;
class MotionBlurRenderer;
class RenderTarget;

class PostProcessor
{
    public:
        PostProcessor(ID3D11RenderTargetView *backBufferTarget);
        ~PostProcessor();

        RenderTarget *getRadianceTarget() const { return this->targets[0]; }

        void render(int width, int height);

    private:
        ID3D11RenderTargetView *backBufferTarget;
        RenderTarget *targets[2]; // two is enough to ping-pong between the targets

        ID3D11VertexShader *postprocessVertexShader;
        ID3D11PixelShader *postprocessPixelShader;

        ID3D11VertexShader *fxaaVertexShader;
        ID3D11PixelShader *fxaaPixelShader;

        Mesh *fullscreenQuad;

        MotionBlurRenderer *motionBlurRenderer;
};
