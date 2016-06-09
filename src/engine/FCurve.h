#pragma once

#include <vector>
#include <engine/glm/glm.hpp>

struct cJSON;
class PropertyMapping;

class FCurve
{
    public:
        FCurve(const cJSON *json);

        void evaluate(float time, const PropertyMapping *properties);

    private:
        struct Keyframe
        {
            glm::vec2 co;
        };

        std::string path;
        int index;
        std::vector<Keyframe> keyframes;
};
