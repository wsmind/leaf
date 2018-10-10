#include <engine/render/Texture.h>

#include <algorithm>

#include <engine/render/Image.h>
#include <engine/render/Shaders.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/FrameGraph.h>
#include <engine/render/graph/Job.h>
#include <engine/render/graph/Pass.h>
#include <engine/resource/ResourceManager.h>

#include <cJSON/cJSON.h>

const std::string Texture::resourceClassName = "Texture";
const std::string Texture::defaultResourceData = "{\"type\": \"IMAGE\", \"image\": \"__default\"}";

void Texture::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    std::string typeString = cJSON_GetObjectItem(json, "type")->valuestring;
    if (typeString == "IMAGE") this->type = TextureType_Image;
    else if (typeString == "ENVIRONMENT_MAP") this->type = TextureType_EnvironmentMap;
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

        case TextureType_EnvironmentMap:
        {
            std::string imageName = cJSON_GetObjectItem(json, "image")->valuestring;
            this->environmentMap = ResourceManager::getInstance()->requestResource<Image>(imageName, this);

            this->environmentMapDirty = true;

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

        case TextureType_EnvironmentMap:
        {
            ResourceManager::getInstance()->releaseResource(this->environmentMap, this);
            this->environmentMap = nullptr;

            if (this->environmentTexture != nullptr)
            {
                this->environmentTexture->Release();
                this->environmentTexture = nullptr;

                this->environmentSRV->Release();
                this->environmentSRV = nullptr;

                for (auto uav : this->environmentUAVs)
                    uav->Release();
                this->environmentUAVs.clear();
            }

            break;
        }
    }
}

void Texture::onResourceUpdated(Resource *resource)
{
    this->environmentMapDirty = true;
}

void Texture::update(FrameGraph *frameGraph)
{
    if (this->environmentMapDirty)
    {
        this->environmentMapDirty = false;

        if (this->environmentTexture != nullptr)
        {
            this->environmentTexture->Release();
            this->environmentTexture = nullptr;

            this->environmentSRV->Release();
            this->environmentSRV = nullptr;

            for (auto uav : this->environmentUAVs)
                uav->Release();
            this->environmentUAVs.clear();
        }

        if (this->environmentMap->getTexture() == nullptr)
            return;

        HRESULT res;

        ID3D11Texture2D *source = nullptr;
        res = this->environmentMap->getTexture()->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&source);
        CHECK_HRESULT(res);

        D3D11_TEXTURE2D_DESC desc;
        source->GetDesc(&desc);
        desc.MipLevels = 0;
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

        res = Device::device->CreateTexture2D(&desc, nullptr, &this->environmentTexture);
        CHECK_HRESULT(res);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = -1;

        res = Device::device->CreateShaderResourceView(this->environmentTexture, &srvDesc, &this->environmentSRV);
        CHECK_HRESULT(res);

        // retrieve the computed number of mips
        this->environmentSRV->GetDesc(&srvDesc);
        int mipLevels = srvDesc.Texture2D.MipLevels;

        for (int i = 0; i < mipLevels; i++)
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            ZeroMemory(&uavDesc, sizeof(uavDesc));
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            uavDesc.Texture2D.MipSlice = i;

            ID3D11UnorderedAccessView *uav = nullptr;
            res = Device::device->CreateUnorderedAccessView(this->environmentTexture, &uavDesc, &uav);
            CHECK_HRESULT(res);

            float roughness = (float)i / (float)(mipLevels - 1);

            int width = std::max(1, (int)desc.Width >> i);
            int height = std::max(1, (int)desc.Height >> i);

            glm::mat4 viewMatrix;
            viewMatrix[0][0] = (float)width;
            viewMatrix[0][1] = (float)height;
            viewMatrix[0][2] = (float)roughness;

            Pass *pass = frameGraph->addPass("GenerateIBL");
            pass->setViewport((float)desc.Width, (float)desc.Height, viewMatrix, glm::mat4(1.0f));

            Batch *batch = pass->addBatch("");
            batch->setResources({ this->environmentMap->getSRV() });
            batch->setUnorderedResources({ uav });
            batch->setComputeShader(Shaders::compute.generateIbl);
            batch->addJob()->addDispatch(width, height, 1);

            this->environmentUAVs.push_back(uav);
        }
    }
}

ID3D11ShaderResourceView *Texture::getSRV() const
{
    switch (this->type)
    {
        case TextureType_Image: return this->image->getSRV(); break;
        case TextureType_EnvironmentMap: return this->environmentSRV; break;
    }

    assert(0);
    return nullptr;
}

int Texture::getMipLevels() const
{
    switch (this->type)
    {
        case TextureType_Image: return this->image->getMipLevels(); break;
        case TextureType_EnvironmentMap: return (int)this->environmentUAVs.size(); break;
    }

    assert(0);
    return 0;
}
