#include <engine/render/RenderList.h>

#include <algorithm>

void RenderList::clear()
{
    this->jobs.clear();
    this->lights.clear();
}

void RenderList::addJob(const Job &job)
{
    this->jobs.push_back(job);
}

void RenderList::addLight(const Light &light)
{
    this->lights.push_back(light);
}

void RenderList::sortFrontToBack(const glm::vec3 &cameraDirection)
{
    std::sort(this->jobs.begin(), this->jobs.end(), [](const Job &lhs, const Job &rhs)[&]
    {
        glm::vec3 lhsPosition = glm::vec3(lhs.transform[3]);
        glm::vec3 rhsPosition = glm::vec3(lhs.transform[3]);

        return glm::dot(cameraDirection, lhsPosition) < glm::dot(cameraDirection, rhsPosition);
    });
}

void RenderList::sortByMaterial()
{
    std::sort(this->jobs.begin(), this->jobs.end(), [](const Job &lhs, const Job &rhs)
    {
        // if material is the same, sort by mesh data
        if (lhs.material == rhs.material)
            return (lhs.subMesh < lhs.subMesh);

        return (lhs.material < rhs.material);
    });
}
