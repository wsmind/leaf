#pragma once

#include <d3d11.h>

struct VertexShaderList
{
    ID3D11VertexShader *background;
    ID3D11VertexShader *basic;
    ID3D11VertexShader *bloom;
    ID3D11VertexShader *depthOnly;
    ID3D11VertexShader *fxaa;
    ID3D11VertexShader *motionBlur;
    ID3D11VertexShader *plop;
    ID3D11VertexShader *postprocess;
    ID3D11VertexShader *standard;
    ID3D11VertexShader *unlit;
};

struct PixelShaderList
{
    ID3D11PixelShader *background;
    ID3D11PixelShader *basic;
    ID3D11PixelShader *bloomThreshold;
    ID3D11PixelShader *bloomDownsample;
    ID3D11PixelShader *bloomAccumulation;
    ID3D11PixelShader *bloomDebug;
    ID3D11PixelShader *depthOnly;
    ID3D11PixelShader *fxaa;
    ID3D11PixelShader *motionBlur;
    ID3D11PixelShader *plop;
    ID3D11PixelShader *postprocess;
    ID3D11PixelShader *standard;
    ID3D11PixelShader *unlit;
};

struct ComputeShaderList
{
    ID3D11ComputeShader *tileMax;
    ID3D11ComputeShader *neighborMax;
    ID3D11ComputeShader *generateIbl;
};

class Shaders
{
    public:
        static void loadShaders();
        static void unloadShaders();

        static VertexShaderList vertex;
        static PixelShaderList pixel;
        static ComputeShaderList compute;
};
