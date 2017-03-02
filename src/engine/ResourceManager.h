#pragma once

#include <cassert>
#include <string>
#include <map>

class Resource;

class ResourceManager
{
    public:
        // this method clones the whole json tree passed to it; it is safe
        // to delete data after the call
        template <class ResourceType>
        void updateResourceData(const std::string &name, const unsigned char *buffer, size_t size);

        template <class ResourceType>
        ResourceType *requestResource(const std::string &name);

        inline void releaseResource(Resource *resource);

        void update();

        // Make sure that all resources that will be unloaded soon
        // are unloaded immediately. This is useful when destroying
        // a subsystem for instance.
        void clearPendingUnloads();

        // dump internal state to the log
        void dumpAllResources(bool loadedOnly) const;

    private:
        static ResourceManager *instance;

        struct ResourceDescriptor
        {
            ResourceDescriptor()
                : resource(nullptr)
                , buffer(nullptr)
                , size(0)
                , users(0)
                , pendingUnload(false)
                , ttl(0)
            {}

            Resource *resource;
            unsigned char *buffer;
            size_t size;
            int users; // refcount, in blender terms

            // resources are not unloaded right away, to avoid fast unload/load cycles
            // ttl is the number of frames left before the actual unload
            bool pendingUnload;
            int ttl;
        };

        ResourceManager();
        ~ResourceManager();

        // if not found, creates a new one
        template <class ResourceType>
        ResourceDescriptor &findDescriptor(const std::string &name);

        typedef std::map<std::string, ResourceDescriptor> DescriptorMap;
        DescriptorMap descriptors;

    public:
        // singleton implementation
        static void create() { assert(!ResourceManager::instance); ResourceManager::instance = new ResourceManager; }
        static void destroy() { assert(ResourceManager::instance); delete ResourceManager::instance; }
        static ResourceManager *getInstance() { assert(ResourceManager::instance); return ResourceManager::instance; }
};

#include <engine/ResourceManager.inline.h>
