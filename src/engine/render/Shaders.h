#pragma once

#include <d3d11.h>

struct VertexShaderList
{
    ID3D11VertexShader *motionBlur;
};

struct PixelShaderList
{
    ID3D11PixelShader *motionBlur;
};

struct ComputeShaderList
{
    ID3D11ComputeShader *tileMax;
    ID3D11ComputeShader *neighborMax;
    ID3D11ComputeShader *generateIbl;
};

struct LayoutList
{
    ID3D11InputLayout *depthOnly;
    ID3D11InputLayout *distanceField;
    ID3D11InputLayout *geometry2D;
    ID3D11InputLayout *instancedMesh;
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
