#include <engine/render/StandardBsdf.h>

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

StandardBsdf::StandardBsdf(cJSON *json)
{
    cJSON *diffuse = cJSON_GetObjectItem(json, "baseColorMultiplier");
    cJSON *emissive = cJSON_GetObjectItem(json, "emissive");
    this->constants.baseColorMultiplier = glm::vec3(cJSON_GetArrayItem(diffuse, 0)->valuedouble, cJSON_GetArrayItem(diffuse, 1)->valuedouble, cJSON_GetArrayItem(diffuse, 2)->valuedouble);
    this->constants.emissive = glm::vec3(cJSON_GetArrayItem(emissive, 0)->valuedouble, cJSON_GetArrayItem(emissive, 1)->valuedouble, cJSON_GetArrayItem(emissive, 2)->valuedouble);
    this->constants.metallicOffset = (float)cJSON_GetObjectItem(json, "metallicOffset")->valuedouble;
    this->constants.roughnessOffset = (float)cJSON_GetObjectItem(json, "roughnessOffset")->valuedouble;

    this->baseColorMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "baseColorMap")->valuestring);
    this->normalMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "normalMap")->valuestring);
    this->metallicMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "metallicMap")->valuestring);
    this->roughnessMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "roughnessMap")->valuestring);

    D3D11_BUFFER_DESC cbDesc;
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.ByteWidth = sizeof(StandardConstants);
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.StructureByteStride = 0;
    cbDesc.MiscFlags = 0;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT res = Device::device->CreateBuffer(&cbDesc, nullptr, &this->constantBuffer);
    CHECK_HRESULT(res);
}

StandardBsdf::~StandardBsdf()
{
    ResourceManager::getInstance()->releaseResource(this->baseColorMap);
    ResourceManager::getInstance()->releaseResource(this->normalMap);
    ResourceManager::getInstance()->releaseResource(this->metallicMap);
    ResourceManager::getInstance()->releaseResource(this->roughnessMap);

    this->constantBuffer->Release();
}

void StandardBsdf::registerAnimatedProperties(PropertyMapping &properties)
{
    properties.add("diffuse_color", (float *)&this->constants.baseColorMultiplier);
    properties.add("leaf.emissive", (float *)&this->constants.emissive);
    properties.add("leaf.metallic_offset", &this->constants.metallicOffset);
    properties.add("leaf.roughness_offset", &this->constants.roughnessOffset);
}

void StandardBsdf::setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants)
{
	this->constants.shadows = *shadowConstants;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = Device::context->Map(this->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &constants, sizeof(constants));
    Device::context->Unmap(this->constantBuffer, 0);

    batch->setVertexShader(Shaders::vertex.standard);
    batch->setPixelShader(Shaders::pixel.standard);

    batch->setShaderConstants(this->constantBuffer);

    batch->setResources({
        this->baseColorMap->getSRV(),
        this->normalMap->getSRV(),
        this->metallicMap->getSRV(),
        this->roughnessMap->getSRV(),
		shadowSRV,
        settings.environment.environmentMap->getSRV()
	});

	batch->setSamplers({
		this->baseColorMap->getSamplerState(),
		this->normalMap->getSamplerState(),
		this->metallicMap->getSamplerState(),
		this->roughnessMap->getSamplerState(),
		shadowSampler,
        this->baseColorMap->getSamplerState() // use base color sampler for envmap
	});
}
