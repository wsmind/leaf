#include <engine/render/Image.h>

#include <engine/resource/ResourceManager.h>

#include <engine/DDSTextureLoader/DDSTextureLoader.h>

const std::string Image::resourceClassName = "Image";
const std::string Image::defaultResourceData = "";

void Image::load(const unsigned char *buffer, size_t size)
{
    DirectX::CreateDDSTextureFromMemory(Device::device, buffer, size, &this->texture, &this->srv);
}

void Image::unload()
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
