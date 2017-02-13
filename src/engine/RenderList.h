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
        };

        struct Light
        {
            glm::vec3 position;
            glm::vec3 color;
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
