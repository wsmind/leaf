#include <engine/Image.h>

#include <engine/ResourceManager.h>

const std::string Image::resourceClassName = "Image";
const std::string Image::defaultResourceData = "{\"width\": 1, \"height\": 1, \"channels\": 4, \"float\": false, \"pixels\": \"null\"}";

void Image::load(const cJSON *json)
{
    int width = cJSON_GetObjectItem(json, "width")->valueint;
    int height = cJSON_GetObjectItem(json, "height")->valueint;
    int channels = cJSON_GetObjectItem(json, "channels")->valueint;
    bool useFloat = (cJSON_GetObjectItem(json, "float")->valueint != 0) ? true : false;
    std::string blobName = cJSON_GetObjectItem(json, "pixels")->valuestring;

    assert(channels == 4);

    int pixelCount = width * height;
    int elementCount = pixelCount * channels;
    int valueSize = useFloat ? sizeof(float) : sizeof(unsigned char);
    int bufferSize = elementCount * valueSize;

    const void *blob = ResourceManager::getInstance()->getBlob(blobName);
    printf("Loading image blob at %p\n", blob);

    const void *buffer = blob;
    if (!blob)
    {
        buffer = malloc(bufferSize);
        memset((void *)buffer, 0, bufferSize);
    }

    /*cJSON *pixels = cJSON_GetObjectItem(json, "pixels");
    cJSON *element = pixels->child;

    int elementCountCheck = 0;
    if (useFloat)
    {
        float *floatBuffer = (float *)buffer;
        while (element)
        {
            *floatBuffer++ = (float)element->valuedouble;
            elementCountCheck++;
            element = element->next;
        }
    }
    else
    {
        unsigned char *byteBuffer = (unsigned char *)buffer;
        while (element)
        {
            *byteBuffer++ = (unsigned char)(element->valuedouble * 255.0);
            elementCountCheck++;
            element = element->next;
        }
    }

    assert(elementCount == elementCountCheck);*/
    
    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = useFloat ? DXGI_FORMAT_R32G32B32A32_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA textureData;
    textureData.pSysMem = buffer;
    textureData.SysMemPitch = width * channels * valueSize;
    textureData.SysMemSlicePitch = 0;

    HRESULT res = Device::device->CreateTexture2D(&textureDesc, &textureData, &this->texture);
    CHECK_HRESULT(res);

    if (!blob)
        free((void *)buffer);

    res = Device::device->CreateShaderResourceView(this->texture, NULL, &this->srv);
    CHECK_HRESULT(res);
}

void Image::unload()
{
    this->texture->Release();
    this->texture = nullptr;

    this->srv->Release();
    this->srv = nullptr;
}
