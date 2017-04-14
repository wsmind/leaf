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
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
};
