#pragma once

#include <windows.h>
#include <d3d11.h>
#include <comdef.h>

#ifdef _DEBUG
    #define CHECK_HRESULT(hr) \
        if (hr != S_OK) \
        { \
            _com_error err(hr); \
            printf("Error: %s\n", err.ErrorMessage()); \
            assert(hr == S_OK); \
        }
#else
    #define CHECK_HRESULT(hr)
#endif

// shallow container for D3D device and context; could be changed to a proper API abstraction someday
class Device
{
    public:
        static ID3D11Device *device;
        static ID3D11DeviceContext *context;
};
