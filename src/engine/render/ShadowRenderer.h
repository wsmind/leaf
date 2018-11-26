#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>

class FrameGraph;
class RenderList;
class Scene;
struct ShadowConstants;

struct DepthOnlyInstanceData
{
    glm::mat4 transformMatrix;
};

class ShadowRenderer
{
    public:
        ShadowRenderer(int resolution);
        ~ShadowRenderer();

        void render(FrameGraph *frameGraph, const Scene *scene, const RenderList *renderList, ID3D11InputLayout *inputLayout);

		ID3D11ShaderResourceView *getSRV() const { return this->srv; }
		ID3D11SamplerState *getSampler() const { return this->sampler; }
        ID3D11Buffer *getConstants() const { return this->constantBuffer; }

    private:
        int resolution;
        ID3D11Texture2D *shadowMap;
        ID3D11DepthStencilView *target;
        ID3D11ShaderResourceView *srv;
        ID3D11SamplerState *sampler;
        ID3D11DepthStencilState *depthState;

        ID3D11Buffer *constantBuffer;
};
