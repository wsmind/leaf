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

void RenderList::sort()
{
    std::sort(this->jobs.begin(), this->jobs.end(), [](const Job &lhs, const Job &rhs)
    {
        // if material is the same, sort by mesh data
        if (lhs.material == rhs.material)
            return (lhs.subMesh < lhs.subMesh);

        return (lhs.material < rhs.material);
    });
}
