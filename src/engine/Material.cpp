#include <engine/Material.h>

#include <engine/AnimationData.h>
#include <engine/AnimationPlayer.h>
#include <engine/Device.h>
#include <engine/PropertyMapping.h>
#include <engine/ResourceManager.h>
#include <engine/Texture.h>

#include <engine/cJSON/cJSON.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"baseColorMultiplier\": [1.0, 1.0, 1.0], \"emissive\": [0.0, 0.0, 0.0], \"metallicOffset\": 0.0, \"roughnessOffset\": 0.0, \"baseColorMap\": \"__default\", \"normalMap\": \"__default\", \"metallicMap\": \"__default\", \"roughnessMap\": \"__default\"}";

void Material::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *diffuse = cJSON_GetObjectItem(json, "baseColorMultiplier");
    cJSON *emissive = cJSON_GetObjectItem(json, "emissive");
    this->data.baseColorMultiplier = glm::vec3(cJSON_GetArrayItem(diffuse, 0)->valuedouble, cJSON_GetArrayItem(diffuse, 1)->valuedouble, cJSON_GetArrayItem(diffuse, 2)->valuedouble);
    this->data.emissive = glm::vec3(cJSON_GetArrayItem(emissive, 0)->valuedouble, cJSON_GetArrayItem(emissive, 1)->valuedouble, cJSON_GetArrayItem(emissive, 2)->valuedouble);
    this->data.metallicOffset = (float)cJSON_GetObjectItem(json, "metallicOffset")->valuedouble;
    this->data.roughnessOffset = (float)cJSON_GetObjectItem(json, "roughnessOffset")->valuedouble;

    this->baseColorMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "baseColorMap")->valuestring);
    this->normalMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "normalMap")->valuestring);
    this->metallicMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "metallicMap")->valuestring);
    this->roughnessMap = ResourceManager::getInstance()->requestResource<Texture>(cJSON_GetObjectItem(json, "roughnessMap")->valuestring);

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        properties.add("diffuse_color", (float *)&this->data.baseColorMultiplier);
        properties.add("leaf.emissive", (float *)&this->data.emissive);
        properties.add("leaf.metallic_offset", &this->data.metallicOffset);
        properties.add("leaf.roughness_offset", &this->data.roughnessOffset);

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }

    cJSON_Delete(json);
}

void Material::unload()
{
    if (this->animation)
    {
        AnimationPlayer::globalPlayer.unregisterAnimation(this->animation);
        delete this->animation;
        this->animation = nullptr;
    }

    ResourceManager::getInstance()->releaseResource(this->baseColorMap);
    ResourceManager::getInstance()->releaseResource(this->normalMap);
    ResourceManager::getInstance()->releaseResource(this->metallicMap);
    ResourceManager::getInstance()->releaseResource(this->roughnessMap);
}

void Material::bindTextures() const
{
    ID3D11SamplerState *samplers[] = {
        this->baseColorMap->getSamplerState(),
        this->normalMap->getSamplerState(),
        this->metallicMap->getSamplerState(),
        this->roughnessMap->getSamplerState()
    };

    ID3D11ShaderResourceView *srvs[] = {
        this->baseColorMap->getSRV(),
        this->normalMap->getSRV(),
        this->metallicMap->getSRV(),
        this->roughnessMap->getSRV()
    };

    Device::context->PSSetSamplers(0, 4, samplers);
    Device::context->PSSetShaderResources(0, 4, srvs);
}
