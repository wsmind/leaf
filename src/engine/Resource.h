#pragma once

class Resource
{
    public:
        virtual ~Resource() {}

        virtual void load(const unsigned char *buffer, size_t size) = 0;
        virtual void unload() = 0;
};
