#include <engine/Material.h>

#include <engine/ResourceManager.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{}";

void Material::load(const cJSON *json)
{
}

void Material::unload()
{
}
