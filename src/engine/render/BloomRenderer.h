#pragma once

#include <d3d11.h>

#include <engine/render/Mesh.h>

class FrameGraph;
struct RenderSettings;
class RenderTarget;

class BloomRenderer
{
    public:
        BloomRenderer(int backbufferWidth, int backbufferHeight);
        ~BloomRenderer();

        void render(FrameGraph *frameGraph, const RenderSettings &settings, RenderTarget *inputTarget, RenderTarget *outputTarget, const Mesh::SubMesh &quadSubMesh);

    private:
		int backbufferWidth;
		int backbufferHeight;

		ID3D11InputLayout *inputLayout;

		static const int DOWNSAMPLE_LEVELS = 8;
		RenderTarget *downsampleTargets[DOWNSAMPLE_LEVELS];
		RenderTarget *blurTargets[DOWNSAMPLE_LEVELS];

		ID3D11Buffer *constantBuffer;
};
