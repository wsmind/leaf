#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <engine/render/Mesh.h>
#include <engine/render/ShaderCache.h>

class Material;

class RenderList
{
    public:
        struct Job
        {
            Material *material;
            const Mesh::SubMesh *subMesh;
            glm::mat4 transform;
            glm::mat4 previousFrameTransform;
        };

        struct DistanceField
        {
            Material *material;
            const Mesh::SubMesh *subMesh;
            glm::mat4 transform;
            glm::mat4 previousFrameTransform;
            ShaderCache::Hash prefixHash;
        };

        struct Light
        {
            bool spot;
            glm::vec3 position;
            float radius;
            glm::vec3 color;

            // only used for spotlights
            glm::vec3 direction;
            float angle;
            float blend;
            glm::mat4 shadowTransform;
            float scattering;
        };

        void clear();

        void addJob(const Job &job);
        void addDistanceField(const DistanceField &distanceField);
        void addLight(const Light &light);
        void sortFrontToBack(const glm::vec3 &cameraDirection);
        void sortByMaterial();

        const std::vector<Job> &getJobs() const { return this->jobs; }
        const std::vector<DistanceField> &getDistanceFields() const { return this->distanceFields; }
        const std::vector<Light> &getLights() const { return this->lights; }

    private:
        std::vector<Job> jobs;
        std::vector<DistanceField> distanceFields;
        std::vector<Light> lights;
};
