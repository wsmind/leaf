#include <engine/render/Mesh.h>

#include <cassert>

#include <engine/render/Material.h>
#include <engine/render/graph/Job.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

const std::string Mesh::resourceClassName = "Mesh";
const std::string Mesh::defaultResourceData = "";

void Mesh::load(const unsigned char *buffer, size_t size)
{
    if (size < sizeof(int))
        return;

    const unsigned char *readPosition = (const unsigned char *)buffer;

    // vertex buffer

    this->vertexCount = *(unsigned int *)readPosition;
    readPosition += sizeof(unsigned int);

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(float) * (3 /* pos */ + 3 /* normal */ + 4 /* tangent */ + 2 /* uv */) * this->vertexCount;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.StructureByteStride = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = readPosition;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HRESULT res = Device::device->CreateBuffer(&vbDesc, &vertexData, &this->vertexBuffer);
    CHECK_HRESULT(res);

    readPosition += vbDesc.ByteWidth;

    unsigned int materialCount = *(unsigned int *)readPosition;
    readPosition += sizeof(unsigned int);

    for (unsigned int i = 0; i < materialCount; i++)
    {
        this->subMeshes.emplace_back();
        SubMesh &subMesh = this->subMeshes.at(i);

        subMesh.vertexBuffer = this->vertexBuffer;

        // material
        unsigned int materialNameSize = *(unsigned int *)readPosition;
        readPosition += sizeof(unsigned int);

        std::string materialName((const char *)readPosition, materialNameSize);
        readPosition += materialNameSize;

        subMesh.material = ResourceManager::getInstance()->requestResource<Material>(materialName);

        // index buffer

        subMesh.indexCount = *(unsigned int *)readPosition;
        readPosition += sizeof(unsigned int);

        D3D11_BUFFER_DESC ibDesc;
        ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
        ibDesc.ByteWidth = sizeof(uint32_t) * subMesh.indexCount;
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibDesc.StructureByteStride = 0;
        ibDesc.MiscFlags = 0;
        ibDesc.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA indexData;
        indexData.pSysMem = readPosition;
        indexData.SysMemPitch = 0;
        indexData.SysMemSlicePitch = 0;

        res = Device::device->CreateBuffer(&ibDesc, &indexData, &subMesh.indexBuffer);
        CHECK_HRESULT(res);

        readPosition += ibDesc.ByteWidth;
    }
}

void Mesh::unload()
{
    if (this->vertexBuffer != nullptr)
    {
        this->vertexBuffer->Release();
        this->vertexBuffer = nullptr;
    }

    this->vertexCount = 0;

    for (auto &subMesh : this->subMeshes)
    {
        subMesh.indexBuffer->Release();
        ResourceManager::getInstance()->releaseResource(subMesh.material);
    }

    this->subMeshes.clear();
}
