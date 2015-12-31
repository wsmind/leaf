#pragma once

#include <string>

class Resource;

class ResourceManager
{
    public:
        Resource *requestResource(const std::string &name);
        void releaseResource(const std::string &name);
};
