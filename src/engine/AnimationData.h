#pragma once

struct cJSON;
class Action;

#include <engine/PropertyMapping.h>

class AnimationData
{
    public:
        AnimationData(const cJSON *json, const PropertyMapping &properties);
        ~AnimationData();

        void update(float time);

    private:
        Action *action;
        PropertyMapping properties;
};
