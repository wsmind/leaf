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

    private:
        int resolution;
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
        ID3D11DepthStencilState *depthState;

        ID3D11VertexShader *depthOnlyVertexShader;
        ID3D11PixelShader *depthOnlyPixelShader;

        ID3D11Buffer *cbDepthOnly;
};
