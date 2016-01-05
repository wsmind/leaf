#include <engine/Material.h>

#include <engine/ResourceManager.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"albedo\": [1.0, 0.0, 1.0], \"metalness\": 0.5, \"roughness\": 0.5}";

void Material::load(const cJSON *json)
{
    cJSON *diffuse = cJSON_GetObjectItem(json, "albedo");
    this->data.albedo = glm::vec3(cJSON_GetArrayItem(diffuse, 0)->valuedouble, cJSON_GetArrayItem(diffuse, 1)->valuedouble, cJSON_GetArrayItem(diffuse, 2)->valuedouble);
    this->data.metalness = (float)cJSON_GetObjectItem(json, "metalness")->valuedouble;
    this->data.roughness = (float)cJSON_GetObjectItem(json, "roughness")->valuedouble;

    // normalize diffuse BRDF
    this->data.albedo /= 3.1415926535f;
}

void Material::unload()
{
}
