#pragma once

#include <d3d11.h>

#include <engine/render/Mesh.h>

class FrameGraph;
class RenderTarget;

class MotionBlurRenderer
{
    public:
        MotionBlurRenderer(int backbufferWidth, int backbufferHeight, int tileSize);
        ~MotionBlurRenderer();

        void render(FrameGraph *frameGraph, RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget, int width, int height, const Mesh::SubMesh &quadSubMesh);

    private:
        int tileSize;
        int tileCountX;
        int tileCountY;

		ID3D11Texture2D *tileMaxTexture;
        ID3D11ShaderResourceView *tileMaxSRV;
        ID3D11UnorderedAccessView *tileMaxUAV;

		ID3D11Texture2D *neighborMaxTexture;
		ID3D11ShaderResourceView *neighborMaxSRV;
		ID3D11UnorderedAccessView *neighborMaxUAV;

		ID3D11SamplerState *neighborMaxSampler;
};
