#pragma once

#include <string>

#include <engine/Device.h>
#include <engine/Resource.h>

class Mesh: public Resource
{
    public:
        static const std::string resourceClassName;

        Mesh() : vertexBuffer(nullptr) {}
        virtual ~Mesh() {}

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        void bind() const;

    private:
        ID3D11Buffer *vertexBuffer;
};
