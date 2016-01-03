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

        void clear();

        void addJob(const Job &job);
        void sort();

        const std::vector<Job> &getJobs() const { return this->jobs; }

    private:
        std::vector<Job> jobs;
};
