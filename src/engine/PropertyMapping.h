#pragma once

#include <string>
#include <map>

class PropertyMapping
{
    public:
        void add(const std::string &name, float *pointer);
        float *get(const std::string &name, int index) const;

    private:
        std::map<std::string, float *> properties;
};
