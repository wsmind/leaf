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

void ResourceManager::registerBlob(const std::string &name, const void *buffer)
{
    printf("got blob %s at %p\n", name.c_str(), buffer);
    this->blobs[name] = buffer;
}

const void *ResourceManager::getBlob(const std::string &name) const
{
    BlobMap::const_iterator it = this->blobs.find(name);

    if (it == this->blobs.end())
        return nullptr;

    return it->second;
}
