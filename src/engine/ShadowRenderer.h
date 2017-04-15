#pragma once

#include <d3d11.h>

class RenderList;
class Scene;

class ShadowRenderer
{
    public:
        ShadowRenderer(int resolution);
        ~ShadowRenderer();

        void render(const Scene *scene, const RenderList *renderList);

        void bind();
        void unbind();

    private:
        int resolution;
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
        ID3D11ShaderResourceView *srv;
        ID3D11SamplerState *sampler;
        ID3D11DepthStencilState *depthState;

        ID3D11VertexShader *depthOnlyVertexShader;
        ID3D11PixelShader *depthOnlyPixelShader;

        ID3D11Buffer *cbDepthOnly;
        ID3D11Buffer *cbShadows;
};
