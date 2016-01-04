#include <engine/Material.h>

#include <engine/ResourceManager.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"diffuse\": [1.0, 0.0, 1.0]}";

void Material::load(const cJSON *json)
{
    cJSON *diffuse = cJSON_GetObjectItem(json, "diffuse");
    this->data.diffuse = glm::vec3(cJSON_GetArrayItem(diffuse, 0)->valuedouble, cJSON_GetArrayItem(diffuse, 1)->valuedouble, cJSON_GetArrayItem(diffuse, 2)->valuedouble);
}

void Material::unload()
{
}
