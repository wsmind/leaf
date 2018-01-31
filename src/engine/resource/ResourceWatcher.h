#pragma once

class Resource;

class ResourceWatcher
{
    public:
        virtual ~ResourceWatcher() {}

        virtual void onResourceUpdated(Resource *resource) = 0;
};
