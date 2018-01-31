#include <engine/resource/ResourceManager.h>

#include <engine/resource/ResourceWatcher.h>

ResourceManager *ResourceManager::instance = nullptr;

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    this->clearPendingUnloads();

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

void ResourceManager::clearPendingUnloads()
{
    // force unload of pending resources
    int pendingCount = 0;
    do
    {
        pendingCount = 0;

        for (auto &it : this->descriptors)
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
}

void ResourceManager::dumpAllResources(bool loadedOnly) const
{
    if (loadedOnly)
        printf("Loaded resources:\n");
    else
        printf("All resources:\n");

    for (auto &it : this->descriptors)
    {
        const std::string &name = it.first;
        const ResourceDescriptor &descriptor = it.second;

        const bool loaded = descriptor.users || descriptor.pendingUnload;
        if (!loaded && loadedOnly)
            continue;

        printf("  %s (%d users, %d bytes%s)\n", name.c_str(), (int)descriptor.users, (int)descriptor.size, descriptor.pendingUnload ? ", pending unload" : "");
    }
}

void ResourceManager::notifyWatchers(const ResourceDescriptor &descriptor) const
{
    for (auto watcher : descriptor.watchers)
        watcher->onResourceUpdated(descriptor.resource);
}
