#pragma once

#include <algorithm>
#include <engine/cJSON/cJSON.h>

#include <engine/Resource.h>

template <class ResourceType>
void ResourceManager::updateResourceData(const std::string &name, const cJSON *data)
{
    ResourceDescriptor &descriptor = findDescriptor<ResourceType>(name);

    if (descriptor.data != nullptr)
        cJSON_Delete(descriptor.data);

    descriptor.data = cJSON_Duplicate(data, 1);

    // reload if necessary
    if (descriptor.users > 0)
    {
        Resource *resource = descriptor.resource;
        cJSON *newData = descriptor.data;
        resource->unload();
        resource->load(newData);
    }
}

template <class ResourceType>
ResourceType *ResourceManager::requestResource(const std::string &name)
{
    ResourceDescriptor &descriptor = findDescriptor<ResourceType>(name);
    ResourceType *resource = static_cast<ResourceType *>(descriptor.resource);

    descriptor.users++;

    // load if needed
    if (descriptor.users == 1)
        resource->load(descriptor.data);

    return resource;
}

void ResourceManager::releaseResource(Resource *resource)
{
    auto it = std::find_if(this->descriptors.begin(), this->descriptors.end(), [resource](const std::pair<std::string, ResourceDescriptor> &tuple) -> bool
    {
        return tuple.second.resource == resource;
    });

    assert(it != this->descriptors.end());
    ResourceDescriptor &descriptor = it->second;

    descriptor.users--;

    // unload if needed
    if (descriptor.users == 0)
        descriptor.resource->unload();
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
        descriptor.data = cJSON_Parse(ResourceType::defaultResourceData.c_str());
    }

    return descriptor;
}
