#include <engine/ResourceManager.h>

ResourceManager *ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // destroy all the resource objects and data
    std::for_each(this->descriptors.begin(), this->descriptors.end(), [](DescriptorMap::value_type &tuple)
    {
        ResourceDescriptor &descriptor = tuple.second;

        assert(descriptor.users == 0);
        delete descriptor.resource;
        cJSON_Delete(descriptor.data);
    });
}
