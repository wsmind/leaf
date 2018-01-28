#include <engine/render/EnvironmentMap.h>

#include <engine/resource/ResourceManager.h>

#include <engine/DDSTextureLoader/DDSTextureLoader.h>

const std::string EnvironmentMap::resourceClassName = "EnvironmentMap";
const std::string EnvironmentMap::defaultResourceData = "";

void EnvironmentMap::load(const unsigned char *buffer, size_t size)
{
    DirectX::CreateDDSTextureFromMemory(Device::device, buffer, size, &this->texture, &this->srv);
}

void EnvironmentMap::unload()
{
    if (this->texture != nullptr)
    {
        this->texture->Release();
        this->texture = nullptr;
    }

    if (this->srv != nullptr)
    {
        this->srv->Release();
        this->srv = nullptr;
    }
}
