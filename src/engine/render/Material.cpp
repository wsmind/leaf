#include <engine/render/Material.h>

#include <algorithm>
#include <iterator>

#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>
#include <engine/render/Bsdf.h>
#include <engine/render/Image.h>
#include <engine/render/RenderSettings.h>
#include <engine/render/StandardBsdf.h>
#include <engine/render/Texture.h>
#include <engine/render/UnlitBsdf.h>
#include <engine/render/graph/Batch.h>
#include <engine/resource/ResourceManager.h>

#include <cJSON/cJSON.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"bsdf\": \"UNLIT\", \"emissive\": [4.0, 0.0, 3.0], \"emissiveMap\": \"__default_white\", \"uvScale\": [1.0, 1.0], \"uvOffset\": [0.0, 0.0]}";

void Material::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    const char *bsdfName = cJSON_GetObjectItem(json, "bsdf")->valuestring;

    if (!strcmp(bsdfName, "STANDARD")) this->bsdf = new StandardBsdf(json);
    if (!strcmp(bsdfName, "UNLIT")) this->bsdf = new UnlitBsdf(json);
    assert(this->bsdf != nullptr);

    cJSON *animation = cJSON_GetObjectItem(json, "animation");
    if (animation)
    {
        PropertyMapping properties;
        this->bsdf->registerAnimatedProperties(properties);

        this->animation = new AnimationData(animation, properties);
        AnimationPlayer::globalPlayer.registerAnimation(this->animation);
    }

    cJSON *prefix = cJSON_GetObjectItem(json, "shaderPrefix");
    if (prefix != nullptr)
    {
        const char *prefixCode = cJSON_GetObjectItem(prefix, "code")->valuestring;
        this->prefixHash = ShaderCache::getInstance()->registerPrefix(prefixCode);

        cJSON *textureList = cJSON_GetObjectItem(prefix, "textures");
        cJSON *textureJson = textureList->child;
        while (textureJson)
        {
            Image *texture = ResourceManager::getInstance()->requestResource<Image>(textureJson->valuestring);
            this->textures.push_back(texture);

            textureJson = textureJson->next;
        }
    }

    cJSON_Delete(json);

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
}

void Material::unload()
{
    this->samplerState->Release();
    this->samplerState = nullptr;

    ShaderCache::getInstance()->unregisterPrefix(this->prefixHash);
    this->prefixHash = { 0, 0 };

    for (auto texture: this->textures)
        ResourceManager::getInstance()->releaseResource(texture);
    this->textures.clear();

    if (this->animation)
    {
        AnimationPlayer::globalPlayer.unregisterAnimation(this->animation);
        delete this->animation;
        this->animation = nullptr;
    }

    delete this->bsdf;
}

void Material::setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ID3D11SamplerState *shadowSampler, ShadowConstants *shadowConstants)
{
    std::vector<ID3D11ShaderResourceView *> resources;
    resources.push_back(shadowSRV);
    resources.push_back(settings.environment.environmentMap->getSRV());
    std::transform(this->textures.begin(), this->textures.end(), std::back_inserter(resources), [](Image *texture) { return texture->getSRV(); });

    batch->setResources(resources);

    std::vector<ID3D11SamplerState *> samplers;
    samplers.push_back(shadowSampler);
    samplers.push_back(this->samplerState); // use base color sampler for envmap
    std::transform(this->textures.begin(), this->textures.end(), std::back_inserter(samplers), [&](Image *texture) { return this->samplerState; });

    batch->setSamplers(samplers);

    this->bsdf->setupBatch(batch, settings, shadowSRV, shadowConstants);
}
