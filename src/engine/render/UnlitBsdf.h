#pragma once

#include <d3d11.h>

#include <engine/render/Bsdf.h>
#include <engine/render/shaders/constants/UnlitConstants.h>

struct cJSON;
struct ShadowConstants;

class UnlitBsdf: public Bsdf
{
    public:
        UnlitBsdf(cJSON *json);
        virtual ~UnlitBsdf();

        virtual void registerAnimatedProperties(PropertyMapping &properties) override;
        virtual void setupBatch(Batch *batch, const RenderSettings &settings, ID3D11ShaderResourceView *shadowSRV, ShadowConstants *shadowConstants) override;

    private:
        UnlitConstants constants;
        ID3D11Buffer *constantBuffer;

        Texture *emissiveMap;
};
