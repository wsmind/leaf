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
        free(descriptor.buffer);
    }
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
