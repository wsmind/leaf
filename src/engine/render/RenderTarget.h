#pragma once

#include <d3d11.h>

class RenderTarget
{
    public:
        RenderTarget(int width, int height, DXGI_FORMAT format, bool msaa = false);
        ~RenderTarget();

		int getWidth() const { return this->width; }
		int getHeight() const { return this->height; }

        ID3D11Texture2D *getTexture() const { return this->texture; }
        ID3D11RenderTargetView *getTarget() const { return this->target; }
        ID3D11SamplerState *getSamplerState() const { return this->samplerState; }
        ID3D11ShaderResourceView *getSRV() const { return this->srv; }

    private:
		int width;
		int height;

        ID3D11Texture2D *texture;
        ID3D11RenderTargetView *target;
        ID3D11SamplerState *samplerState;
        ID3D11ShaderResourceView *srv;
};
