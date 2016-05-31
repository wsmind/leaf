#pragma once

struct cJSON;
class Action;

class AnimationData
{
    public:
       AnimationData(const cJSON *json);
       ~AnimationData();

    private:
        Action *action;
};
