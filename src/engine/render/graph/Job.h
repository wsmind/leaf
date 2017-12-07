#pragma once

#include <d3d11.h>

class Job
{
    public:
        void setBuffers(ID3D11Buffer *vertexBuffer, ID3D11Buffer *indexBuffer, int indexCount)
        {
            this->vertexBuffer = vertexBuffer;
            this->indexBuffer = indexBuffer;
            this->indexCount = indexCount;
        }

        void execute(ID3D11DeviceContext *context);

    private:
        ID3D11Buffer * vertexBuffer = nullptr;
        ID3D11Buffer *indexBuffer = nullptr;
        int indexCount = 0;
};
