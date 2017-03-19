#include <engine/Texture.h>

#include <engine/Image.h>
#include <engine/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

const std::string Texture::resourceClassName = "Texture";
const std::string Texture::defaultResourceData = "{\"type\": \"IMAGE\", \"image\": \"__default\"}";

void Texture::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    std::string typeString = cJSON_GetObjectItem(json, "type")->valuestring;
    if (typeString == "IMAGE") this->type = TextureType_Image;
    else assert(0);

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // trilinear
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Device::device->CreateSamplerState(&samplerDesc, &this->samplerState);

    switch (this->type)
    {
        case TextureType_Image:
        {
            std::string imageName = cJSON_GetObjectItem(json, "image")->valuestring;
            this->image = ResourceManager::getInstance()->requestResource<Image>(imageName);
            break;
        }
    }

    cJSON_Delete(json);
}

void Texture::unload()
{
    this->samplerState->Release();
    this->samplerState = nullptr;

    switch (this->type)
    {
        case TextureType_Image:
        {
            ResourceManager::getInstance()->releaseResource(this->image);
            this->image = nullptr;
            break;
        }
    }
}

ID3D11ShaderResourceView *Texture::getSRV() const
{
    switch (this->type)
    {
        case TextureType_Image: return this->image->getSRV(); break;
    }

    assert(0);
    return nullptr;
}
