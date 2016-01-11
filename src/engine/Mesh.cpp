#include <engine/Mesh.h>

#include <cassert>

#include <engine/Material.h>
#include <engine/ResourceManager.h>

const std::string Mesh::resourceClassName = "Mesh";
const std::string Mesh::defaultResourceData = "{\"vertices\": [-1, -1, 0, 0, 0, 1, 0, 0, -1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1], \"vertexCount\": 3, \"material\": \"__default\"}";

void Mesh::load(const cJSON *json)
{
    cJSON *jsonVertices = cJSON_GetObjectItem(json, "vertices");
    this->vertexCount = cJSON_GetObjectItem(json, "vertexCount")->valueint;

    std::string materialName = cJSON_GetObjectItem(json, "material")->valuestring;
    this->material = ResourceManager::getInstance()->requestResource<Material>(materialName);

    int arraySize = cJSON_GetArraySize(jsonVertices);
    float *vertices = new float[arraySize];

    cJSON *floatValue = jsonVertices->child;
    float *writePosition = vertices;
    while (floatValue)
    {
        *writePosition++ = (float)floatValue->valuedouble;

        floatValue = floatValue->next;
    }

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.ByteWidth = sizeof(float) * arraySize;
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

    delete[] vertices;
}

void Mesh::unload()
{
    this->vertexBuffer->Release();
    this->vertexBuffer = nullptr;

    this->vertexCount = 0;

    ResourceManager::getInstance()->releaseResource(this->material);
}

void Mesh::bind() const
{
    assert(this->vertexBuffer != nullptr);

    UINT stride = sizeof(float) * (3 /* pos */ + 3 /* normal */ + 2 /* uv */);
    UINT offset = 0;
    Device::context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    Device::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
