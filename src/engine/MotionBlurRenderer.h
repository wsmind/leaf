#pragma once

#include <d3d11.h>

class RenderTarget;

class MotionBlurRenderer
{
    public:
        MotionBlurRenderer();
        ~MotionBlurRenderer();

        void render(RenderTarget *radianceTarget, RenderTarget *motionTarget, RenderTarget *outputTarget);

    private:
        ID3D11PixelShader *motionblurPixelShader;

        ID3D11Texture2D *tileMaxTexture;
        ID3D11ShaderResourceView *tileMaxSRV;
        ID3D11UnorderedAccessView *tileMaxUAV;
};
