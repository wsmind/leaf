#include <engine/render/Mesh.h>

#include <cassert>

#include <engine/render/Material.h>
#include <engine/render/graph/Job.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

const std::string Mesh::resourceClassName = "Mesh";
const std::string Mesh::defaultResourceData = "{"
        "\"vertices\": ["
            "-1, -1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0,"
            "-1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1,"
            "1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1"
        "],"
        "\"vertexCount\": 3,"
        "\"indices\": ["
            "0, 1, 2"
        "],"
        "\"indexCount\": 3,"
        "\"material\": \"__default\","
        "\"minBound\": [-1.0, -1.0, 0.0],"
        "\"maxBound\": [1.0, 1.0, 0.0]"
    "}";

void Mesh::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *jsonVertices = cJSON_GetObjectItem(json, "vertices");
    this->vertexCount = cJSON_GetObjectItem(json, "vertexCount")->valueint;

    cJSON *jsonIndices = cJSON_GetObjectItem(json, "indices");
    this->indexCount = cJSON_GetObjectItem(json, "indexCount")->valueint;

    std::string materialName = cJSON_GetObjectItem(json, "material")->valuestring;
    this->material = ResourceManager::getInstance()->requestResource<Material>(materialName);

    cJSON *minBoundJson = cJSON_GetObjectItem(json, "minBound");
    this->minBound = glm::vec3(cJSON_GetArrayItem(minBoundJson, 0)->valuedouble, cJSON_GetArrayItem(minBoundJson, 1)->valuedouble, cJSON_GetArrayItem(minBoundJson, 2)->valuedouble);

    cJSON *maxBoundJson = cJSON_GetObjectItem(json, "maxBound");
    this->maxBound = glm::vec3(cJSON_GetArrayItem(maxBoundJson, 0)->valuedouble, cJSON_GetArrayItem(maxBoundJson, 1)->valuedouble, cJSON_GetArrayItem(maxBoundJson, 2)->valuedouble);

    // vertex buffer

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

    // index buffer

    arraySize = cJSON_GetArraySize(jsonIndices);
    uint32_t *indices = new uint32_t[arraySize];

    cJSON *intValue = jsonIndices->child;
    uint32_t *indexWritePosition = indices;
    while (intValue)
    {
        *indexWritePosition++ = (uint32_t)intValue->valueint;

        intValue = intValue->next;
    }

    D3D11_BUFFER_DESC ibDesc;
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.ByteWidth = sizeof(uint32_t) * arraySize;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.StructureByteStride = 0;
    ibDesc.MiscFlags = 0;
    ibDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    res = Device::device->CreateBuffer(&ibDesc, &indexData, &this->indexBuffer);
    CHECK_HRESULT(res);

    delete[] indices;

    cJSON_Delete(json);
}

void Mesh::unload()
{
    this->vertexBuffer->Release();
    this->vertexBuffer = nullptr;

    this->vertexCount = 0;

    this->indexBuffer->Release();
    this->indexBuffer = nullptr;

    this->indexCount = 0;

    ResourceManager::getInstance()->releaseResource(this->material);
}

void Mesh::bind() const
{
    assert(this->vertexBuffer != nullptr);

    UINT stride = sizeof(float) * (3 /* pos */ + 3 /* normal */ + 4 /* tangent */ + 2 /* uv */);
    UINT offset = 0;
    Device::context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    Device::context->IASetIndexBuffer(this->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    Device::context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Mesh::setupJob(Job *job) const
{
    assert(this->vertexBuffer != nullptr);

    job->setBuffers(this->vertexBuffer, this->indexBuffer, this->indexCount);
}
