#pragma once

#include <string>

#include <engine/Device.h>
#include <engine/Resource.h>

class Mesh: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Mesh() : vertexBuffer(nullptr), vertexCount(0) {}
        virtual ~Mesh() {}

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        void bind() const;
        int getVertexCount() const;

    private:
        ID3D11Buffer *vertexBuffer;
        int vertexCount;
};
