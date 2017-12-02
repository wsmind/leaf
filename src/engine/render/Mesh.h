#pragma once

#include <string>

#include <engine/render/Device.h>
#include <engine/resource/Resource.h>

#include <engine/glm/vec3.hpp>

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

        int getIndexCount() const { return this->indexCount; }
        Material *getMaterial() const { return this->material; }

    private:
        ID3D11Buffer *vertexBuffer;
        int vertexCount;

        ID3D11Buffer *indexBuffer;
        int indexCount;

        // assume one material per mesh; submeshes may be implemented later
        Material *material;

        // AABB
        glm::vec3 minBound;
        glm::vec3 maxBound;
};
