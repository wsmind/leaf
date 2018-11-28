#pragma once

#include <d3d11.h>

#include <engine/render/graph/Batch.h>

struct SlangCompileRequest;

class ShaderVariant
{
    public:
        ShaderVariant(SlangCompileRequest *slangRequest, int translationUnitIndex);
        ~ShaderVariant();

        Pipeline getPipeline() const { return this->pipeline; }

    private:
        Pipeline pipeline;
};
