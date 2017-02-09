#pragma once

#include <string>

#include <engine/Device.h>
#include <engine/Resource.h>

class Material;

class Mesh: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        Mesh(): vertexBuffer(nullptr), vertexCount(0) {}
        virtual ~Mesh() {}

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        void bind() const;

        int getVertexCount() const { return this->vertexCount; }
        Material *getMaterial() const { return this->material; }

    private:
        ID3D11Buffer *vertexBuffer;
        int vertexCount;

        // assume one material per mesh; submeshes may be implemented later
        Material *material;
};
