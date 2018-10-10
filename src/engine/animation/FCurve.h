#pragma once

#include <vector>
#include <glm/glm.hpp>

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
            int interpolation;
            glm::vec2 co;
            glm::vec2 leftHandle;
            glm::vec2 rightHandle;
        };

        static float interpolateConstant(const Keyframe &a, const Keyframe &b, float time);
        static float interpolateLinear(const Keyframe &a, const Keyframe &b, float time);
        static float interpolateBezier(const Keyframe &a, const Keyframe &b, float time);

        typedef float(*KeyframeInterpolator)(const Keyframe &a, const Keyframe &b, float time);
        static KeyframeInterpolator interpolators[3];

        std::string path;
        int index;
        std::vector<Keyframe> keyframes;
};
