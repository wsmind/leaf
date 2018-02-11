#include <engine/render/Material.h>

#include <engine/animation/AnimationData.h>
#include <engine/animation/AnimationPlayer.h>
#include <engine/animation/PropertyMapping.h>
#include <engine/render/Bsdf.h>
#include <engine/render/StandardBsdf.h>
#include <engine/render/UnlitBsdf.h>

#include <engine/cJSON/cJSON.h>

const std::string Material::resourceClassName = "Material";
const std::string Material::defaultResourceData = "{\"bsdf\": \"UNLIT\", \"emissive\": [4.0, 0.0, 3.0], \"emissiveMap\": \"__default_white\"}";

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

    cJSON_Delete(json);
}

void Material::unload()
{
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
    this->bsdf->setupBatch(batch, settings, shadowSRV, shadowSampler, shadowConstants);
}
