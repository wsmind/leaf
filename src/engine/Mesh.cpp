#include <engine/Mesh.h>

#include <cassert>
#include <engine/ResourceManager.h>

const std::string Mesh::resourceClassName = "Mesh";

void Mesh::load(const cJSON *json)
{
    float vertices[] = {
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, -1.0f,
        1.0f, 1.0f
    };

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.StructureByteStride = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HRESULT res = Device::device->CreateBuffer(&vbDesc, &vertexData, &this->vertexBuffer);
    CHECK_HRESULT(res);

    // load indices
    // device->CreateIndexBuffer()
}

void Mesh::unload()
{
    this->vertexBuffer->Release();
    this->vertexBuffer = nullptr;
}

void Mesh::bind() const
{
    assert(this->vertexBuffer != nullptr);

    UINT stride = sizeof(float) * 2;
    UINT offset = 0;
    Device::context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    Device::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
