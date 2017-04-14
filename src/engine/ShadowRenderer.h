#pragma once

#include <d3d11.h>

class Scene;

class ShadowRenderer
{
    public:
        ShadowRenderer(int resolution);
        ~ShadowRenderer();

        void render(const Scene *scene);

    private:
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
};
