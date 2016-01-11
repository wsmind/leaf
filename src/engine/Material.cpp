#include <engine/Material.h>

#include <engine/Device.h>
#include <engine/ResourceManager.h>
#include <engine/Texture.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"albedo\": [1.0, 0.0, 1.0], \"metalness\": 0.5, \"roughness\": 0.5}";

void Material::load(const cJSON *json)
{
    cJSON *diffuse = cJSON_GetObjectItem(json, "albedo");
    this->data.albedo = glm::vec3(cJSON_GetArrayItem(diffuse, 0)->valuedouble, cJSON_GetArrayItem(diffuse, 1)->valuedouble, cJSON_GetArrayItem(diffuse, 2)->valuedouble);
    this->data.metalness = (float)cJSON_GetObjectItem(json, "metalness")->valuedouble;
    this->data.roughness = (float)cJSON_GetObjectItem(json, "roughness")->valuedouble;

    this->tex = ResourceManager::getInstance()->requestResource<Texture>("Tex");
}

void Material::unload()
{
    ResourceManager::getInstance()->releaseResource(this->tex);
}

void Material::bindTextures() const
{
    ID3D11SamplerState *sampler = this->tex->getSamplerState();
    ID3D11ShaderResourceView *srv = this->tex->getSRV();

    Device::context->PSSetSamplers(0, 1, &sampler);
    Device::context->PSSetShaderResources(0, 1, &srv);
}
