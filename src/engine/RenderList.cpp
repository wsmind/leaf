#include <engine/RenderList.h>

#include <algorithm>

void RenderList::clear()
{
    this->jobs.clear();
}

void RenderList::addJob(const Job &job)
{
    this->jobs.push_back(job);
}

void RenderList::sort()
{
    std::sort(this->jobs.begin(), this->jobs.end(), [](const Job &lhs, const Job &rhs)
    {
        return (lhs.material < rhs.material) && (lhs.mesh < lhs.mesh);
    });
}
