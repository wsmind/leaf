#include <engine/PropertyMapping.h>

#include <cassert>

void PropertyMapping::add(const std::string &name, float *pointer)
{
    assert(this->properties.find(name) == this->properties.end());

    this->properties[name] = pointer;
}

float *PropertyMapping::get(const std::string &name, int index) const
{
    const auto &it = this->properties.find(name);
    if (it == this->properties.end())
        return nullptr;

    return it->second + index;
}
