#include <engine/Texture.h>

#include <engine/ResourceManager.h>

const std::string Texture::resourceClassName = "Texture";
const std::string Texture::defaultResourceData = "{\"type\": \"IMAGE\", \"image\": \"__default\"}";

void Texture::load(const cJSON *json)
{
}

void Texture::unload()
{
}
