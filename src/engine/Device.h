#pragma once

#include <windows.h>
#include <d3d11.h>

// shallow container for D3D device and context; could be changed to a proper API abstraction someday
class Device
{
    public:
        static ID3D11Device *device;
        static ID3D11DeviceContext *context;
};
