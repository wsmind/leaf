#pragma once

#include <vector>

#include <engine/glm/glm.hpp>

class Material;
class Mesh;

class RenderList
{
    public:
        struct Job
        {
            Material *material;
            Mesh *mesh;
            glm::mat4 transform;
            glm::mat4 previousFrameTransform;
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
        void addLight(const Light &light);
        void sort();

        const std::vector<Job> &getJobs() const { return this->jobs; }
        const std::vector<Light> &getLights() const { return this->lights; }

    private:
        std::vector<Job> jobs;
        std::vector<Light> lights;
};
