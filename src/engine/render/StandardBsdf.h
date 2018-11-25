#pragma once

#include <d3d11.h>

#include <engine/render/Bsdf.h>
#include <engine/render/shaders/constants/StandardConstants.h>

struct cJSON;

class StandardBsdf: public Bsdf
{
    public:
        StandardBsdf(cJSON *json);
        virtual ~StandardBsdf();

        virtual void registerAnimatedProperties(PropertyMapping &properties) override;
        virtual void setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ShadowConstants *shadowConstants) override;

    private:
        StandardConstants constants;
        ID3D11Buffer *constantBuffer;

        Texture *baseColorMap;
        Texture *normalMap;
        Texture *metallicMap;
        Texture *roughnessMap;
};
