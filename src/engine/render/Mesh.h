#pragma once

#include <string>
#include <vector>

#include <engine/render/Device.h>
#include <engine/resource/Resource.h>

#include <glm/vec3.hpp>

class Job;
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

        struct SubMesh
        {
            ID3D11Buffer *vertexBuffer; // same VB as the whole mesh
            ID3D11Buffer *indexBuffer; // separate IB per submesh
            int indexCount;
            Material *material;

            SubMesh()
                : vertexBuffer(nullptr)
                , indexBuffer(nullptr)
                , indexCount(0)
                , material(nullptr)
            {}
        };

        const std::vector<SubMesh> &getSubMeshes() const { return this->subMeshes; }

    private:
        ID3D11Buffer *vertexBuffer;
        int vertexCount;

        std::vector<SubMesh> subMeshes;

        // AABB
        glm::vec3 minBound;
        glm::vec3 maxBound;
};
