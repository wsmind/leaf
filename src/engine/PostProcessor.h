#pragma once

#include <d3d11.h>

class Mesh;
class RenderTarget;

class PostProcessor
{
    public:
        PostProcessor(ID3D11RenderTargetView *backBufferTarget);
        ~PostProcessor();

        RenderTarget *getRadianceTarget() const { return this->radianceTarget; }

        void render(int width, int height);

    private:
        ID3D11RenderTargetView *backBufferTarget;
        RenderTarget *radianceTarget;

        ID3D11VertexShader *postprocessVertexShader;
        ID3D11PixelShader *postprocessPixelShader;

        Mesh *fullscreenQuad;
};
