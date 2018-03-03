#pragma once

#include <algorithm>

#include <engine/resource/Resource.h>
#include <engine/resource/ResourceWatcher.h>

template <class ResourceType>
void ResourceManager::updateResourceData(const std::string &name, const unsigned char *buffer, size_t size)
{
    ResourceDescriptor &descriptor = findDescriptor<ResourceType>(name);

    if (descriptor.buffer != nullptr)
        free(descriptor.buffer);

    descriptor.buffer = (unsigned char *)malloc(size);
    descriptor.size = size;
    memcpy(descriptor.buffer, buffer, size);

    // reload if necessary
    if ((descriptor.users > 0) || descriptor.pendingUnload)
    {
        Resource *resource = descriptor.resource;
        const unsigned char *newBuffer = descriptor.buffer; // calling unload() could indirectly move descriptors in memory, so we keep a copy of that pointer
        resource->unload();
        resource->load(newBuffer, size);

        this->notifyWatchers(descriptor);
    }
}

template <class ResourceType>
ResourceType *ResourceManager::requestResource(const std::string &name, ResourceWatcher *watcher)
{
    ResourceDescriptor &descriptor = findDescriptor<ResourceType>(name);
    ResourceType *resource = static_cast<ResourceType *>(descriptor.resource);

    descriptor.users++;

    if (watcher != nullptr)
    {
        assert(std::find(descriptor.watchers.begin(), descriptor.watchers.end(), watcher) == descriptor.watchers.end());
        descriptor.watchers.push_back(watcher);
    }

    // load if needed
    if (descriptor.pendingUnload)
        descriptor.pendingUnload = false;
    else if (descriptor.users == 1)
        resource->load(descriptor.buffer, descriptor.size);

    return resource;
}

void ResourceManager::releaseResource(Resource *resource, ResourceWatcher *watcher)
{
    auto it = std::find_if(this->descriptors.begin(), this->descriptors.end(), [resource](const std::pair<std::string, ResourceDescriptor> &tuple) -> bool
    {
        return tuple.second.resource == resource;
    });

    assert(it != this->descriptors.end());
    ResourceDescriptor &descriptor = it->second;

    descriptor.users--;

    if (watcher != nullptr)
    {
        auto it = std::find(descriptor.watchers.begin(), descriptor.watchers.end(), watcher);
        assert(it != descriptor.watchers.end());

        descriptor.watchers.erase(it);
    }

    // unload if needed
    if (descriptor.users == 0)
    {
        descriptor.pendingUnload = true;
        descriptor.ttl = 20; // keep it around for 20 frames
    }
}

template <class ResourceType>
ResourceManager::ResourceDescriptor &ResourceManager::findDescriptor(const std::string &name)
{
    // resource ID is Type_Name; e.g: Mesh_Door
    const std::string resourceId = ResourceType::resourceClassName + "_" + name;

    // this will create a new descriptor if not found
    ResourceDescriptor &descriptor = this->descriptors[resourceId];

    // if it's a new descriptor, we also need to instanciate the actual resource object
    if (descriptor.resource == nullptr)
    {
        descriptor.resource = new ResourceType;
        descriptor.buffer = (unsigned char *)malloc(ResourceType::defaultResourceData.size());
        descriptor.size = ResourceType::defaultResourceData.size();
        memcpy(descriptor.buffer, ResourceType::defaultResourceData.c_str(), descriptor.size);
    }

    return descriptor;
}
