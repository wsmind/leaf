#pragma once

#include <string>

#include <windows.h>
#include <d3d11.h>

class Pass
{
    public:
        Pass(const std::string &name);
        ~Pass();

        void execute(ID3D11DeviceContext *context);

    private:
        std::string name;
};
