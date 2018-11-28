#pragma once

#include <d3d11.h>

struct VertexShaderList
{
    ID3D11VertexShader *basic;
    ID3D11VertexShader *bloom;
    ID3D11VertexShader *fxaa;
    ID3D11VertexShader *motionBlur;
    ID3D11VertexShader *plop;
};

struct PixelShaderList
{
    ID3D11PixelShader *basic;
    ID3D11PixelShader *bloomThreshold;
    ID3D11PixelShader *bloomDownsample;
    ID3D11PixelShader *bloomAccumulation;
    ID3D11PixelShader *bloomDebug;
    ID3D11PixelShader *fxaa;
    ID3D11PixelShader *motionBlur;
    ID3D11PixelShader *plop;
};

struct ComputeShaderList
{
    ID3D11ComputeShader *tileMax;
    ID3D11ComputeShader *neighborMax;
    ID3D11ComputeShader *generateIbl;
};

struct LayoutList
{
    ID3D11InputLayout *instancedMesh;
    ID3D11InputLayout *depthOnly;
};

class Shaders
{
    public:
        static void loadShaders();
        static void unloadShaders();

        static VertexShaderList vertex;
        static PixelShaderList pixel;
        static ComputeShaderList compute;

        static LayoutList layout;
};
