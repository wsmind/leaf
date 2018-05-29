#include <engine/render/UnlitBsdf.h>

#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>
#include <engine/render/Device.h>
#include <engine/render/RenderSettings.h>
#include <engine/render/Shaders.h>
#include <engine/render/Texture.h>
#include <engine/render/graph/Batch.h>
#include <engine/resource/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

UnlitBsdf::UnlitBsdf(cJSON *json)
{
    cJSON *emissive = cJSON_GetObjectItem(json, "emissive");
    this->constants.emissive = glm::vec3(cJSON_GetArrayItem(emissive, 0)->valuedouble, cJSON_GetArrayItem(emissive, 1)->valuedouble, cJSON_GetArrayItem(emissive, 2)->valuedouble);

    cJSON *uvScale = cJSON_GetObjectItem(json, "uvScale");
    this->constants.uvScale = glm::vec2(cJSON_GetArrayItem(uvScale, 0)->valuedouble, cJSON_GetArrayItem(uvScale, 1)->valuedouble);

    cJSON *uvOffset = cJSON_GetObjectItem(json, "uvOffset");
    this->constants.uvOffset = glm::vec2(cJSON_GetArrayItem(uvOffset, 0)->valuedouble, cJSON_GetArrayItem(uvOffset, 1)->valuedouble);

    this->emissiveMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "emissiveMap")->valuestring);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(UnlitConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT res = Device::device->CreateBuffer(&cbDesc, nullptr, &this->constantBuffer);
    CHECK_HRESULT(res);
}

UnlitBsdf::~UnlitBsdf()
{
    ResourceManager::getInstance()->releaseResource(this->emissiveMap);

    this->constantBuffer->Release();
}

void UnlitBsdf::registerAnimatedProperties(PropertyMapping &properties)
{
    properties.add("leaf.emissive", (float *)&this->constants.emissive);
    properties.add("leaf.uv_scale", (float *)&this->constants.uvScale);
    properties.add("leaf.uv_offset", (float *)&this->constants.uvOffset);
}

void UnlitBsdf::setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &constants, sizeof(constants));
    Device::context->Unmap(this->constantBuffer, 0);

    batch->setVertexShader(Shaders::vertex.unlit);
    batch->setPixelShader(Shaders::pixel.unlit);

    batch->setShaderConstants(this->constantBuffer);

    batch->setResources({
        this->emissiveMap->getSRV(),
	});

	batch->setSamplers({
		this->emissiveMap->getSamplerState(),
	});
}
