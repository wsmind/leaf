#include <engine/Mesh.h>

#include <cassert>

const char *Mesh::resourceClassName = "Mesh";

Mesh::Mesh()
    : vertexBuffer(nullptr)
{
}

void Mesh::load(const cJSON *json)
{
    // load vertices
    // device->CreateVertexBuffer()

    // load indices
    // device->CreateIndexBuffer()

    // create input layout
}

void Mesh::unload()
{
}

void Mesh::bind() const
{
    assert(this->vertexBuffer != nullptr);

    //Device::context->IASetInputLayout(this->inputLayout);

    UINT stride = sizeof(float) * 2;
    UINT offset = 0;
    Device::context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    Device::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
