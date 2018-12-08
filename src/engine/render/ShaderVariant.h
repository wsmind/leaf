#pragma once

#include <d3d11.h>

#include <engine/render/graph/Batch.h>

struct SlangCompileRequest;

class ShaderVariant
{
    public:
        ~ShaderVariant();

        void compileShaders(SlangCompileRequest *slangRequest, int translationUnitIndex, FILE *exportStream = nullptr);
        void loadShaders(FILE *inputStream);

        Pipeline getPipeline() const { return this->pipeline; }

    private:
        Pipeline pipeline;
};
