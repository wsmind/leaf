#include <engine/ResourceManager.h>

ResourceManager *ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // force unload of pending resources
    int pendingCount = 0;
    do
    {
        pendingCount = 0;

        for (auto &it: this->descriptors)
        {
            ResourceDescriptor &descriptor = it.second;
            if (descriptor.pendingUnload)
            {
                descriptor.resource->unload();
                descriptor.pendingUnload = false;

                pendingCount++;
            }
        }
    } while (pendingCount > 0);

    // destroy all the resource objects and data
    for (auto &it: this->descriptors)
    {
        ResourceDescriptor &descriptor = it.second;

        assert(descriptor.users == 0);
        delete descriptor.resource;
        cJSON_Delete(descriptor.data);
    }
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

void ResourceManager::update()
{
    for (auto &it: this->descriptors)
    {
        ResourceDescriptor &descriptor = it.second;
        if (descriptor.pendingUnload)
        {
            descriptor.ttl--;

            // when the number of frames has passed, actually unload the resource
            if (descriptor.ttl == 0)
            {
                descriptor.resource->unload();
                descriptor.pendingUnload = false;
            }
        }
    }
}
