#pragma once

#include <d3d11.h>

class FrameGraph;
class Mesh;
class RenderTarget;

class MotionBlurRenderer
{
    public:
        MotionBlurRenderer(int backbufferWidth, int backbufferHeight, int tileSize);
        ~MotionBlurRenderer();

        void render(FrameGraph *frameGraph, RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget, int width, int height, Mesh *quad);

    private:
        int tileSize;
        int tileCountX;
        int tileCountY;

		ID3D11VertexShader *motionblurVertexShader;
		ID3D11PixelShader *motionblurPixelShader;
        ID3D11ComputeShader *tileMaxComputeShader;
		ID3D11ComputeShader *neighborMaxComputeShader;

		ID3D11InputLayout *inputLayout;

		ID3D11Texture2D *tileMaxTexture;
        ID3D11ShaderResourceView *tileMaxSRV;
        ID3D11UnorderedAccessView *tileMaxUAV;

		ID3D11Texture2D *neighborMaxTexture;
		ID3D11ShaderResourceView *neighborMaxSRV;
		ID3D11UnorderedAccessView *neighborMaxUAV;

		ID3D11SamplerState *neighborMaxSampler;
};
