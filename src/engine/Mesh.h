#pragma once

#include <engine/Device.h>
#include <engine/Resource.h>

class Mesh: public Resource
{
    public:
        static const char *resourceClassName;

        Mesh();

        virtual void load(const cJSON *json) override;
        virtual void unload() override;

        void bind() const;

    private:
        ID3D11Buffer *vertexBuffer;
};
