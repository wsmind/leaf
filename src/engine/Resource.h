#pragma once

struct cJSON;

class Resource
{
    public:
        virtual void load(const cJSON *json) = 0;
        virtual void unload() = 0;
};
