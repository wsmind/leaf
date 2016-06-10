#include <engine/FCurve.h>

#include <engine/cJSON/cJSON.h>
#include <engine/PropertyMapping.h>

FCurve::FCurve(const cJSON *json)
{
    this->path = std::string(cJSON_GetObjectItem(json, "path")->valuestring);
    this->index = cJSON_GetObjectItem(json, "index")->valueint;

    cJSON *keyframeData = cJSON_GetObjectItem(json, "keyframes")->child;
    while (keyframeData)
    {
        Keyframe key;
        key.interpolation = cJSON_GetArrayItem(keyframeData, 0)->valueint;
        key.co.x = (float)cJSON_GetArrayItem(keyframeData, 1)->valuedouble;
        key.co.y = (float)cJSON_GetArrayItem(keyframeData, 2)->valuedouble;
        key.leftHandle.x = (float)cJSON_GetArrayItem(keyframeData, 3)->valuedouble;
        key.leftHandle.y = (float)cJSON_GetArrayItem(keyframeData, 4)->valuedouble;
        key.rightHandle.x = (float)cJSON_GetArrayItem(keyframeData, 5)->valuedouble;
        key.rightHandle.y = (float)cJSON_GetArrayItem(keyframeData, 6)->valuedouble;
        this->keyframes.push_back(key);

        keyframeData = keyframeData->next;
    }
}

void FCurve::evaluate(float time, const PropertyMapping *properties)
{
    float *property = properties->get(this->path, this->index);
    if (!property)
        return;
    
    if (this->keyframes.size() == 0)
        return;

    // time before first keyframe
    if (time < this->keyframes[0].co.x)
    {
        *property = this->keyframes[0].co.y;
        return;
    }

    // time after last keyframe
    if (time >= this->keyframes[this->keyframes.size() - 1].co.x)
    {
        *property = this->keyframes[this->keyframes.size() - 1].co.y;
        return;
    }

    int i = 1;
    while (time >= this->keyframes[i].co.x)
        i++;

    i--;
    const Keyframe &a = this->keyframes[i];
    const Keyframe &b = this->keyframes[i + 1];
    KeyframeInterpolator interpolator = FCurve::interpolators[a.interpolation];
    *property = interpolator(a, b, time);
}

float FCurve::interpolateConstant(const Keyframe &a, const Keyframe &b, float time)
{
    return a.co.y;
}

float FCurve::interpolateLinear(const Keyframe &a, const Keyframe &b, float time)
{
    // basic lerp
    float t = (time - a.co.x) / (b.co.x - a.co.x);
    return a.co.y + (b.co.y - a.co.y) * t;
}

float FCurve::interpolateBezier(const Keyframe &a, const Keyframe &b, float time)
{
    float t = (time - a.co.x) / (b.co.x - a.co.x);
    float tt = t * t;
    float ttt = t * tt;
    float u = 1.0f - t;
    float uu = u * u;
    float uuu = u * uu;

    return uuu * a.co.y + 3.0f * uu * t * a.rightHandle.y + 3.0f * u * tt * b.leftHandle.y + ttt * b.co.y;
}

FCurve::KeyframeInterpolator FCurve::interpolators[3] = {
    FCurve::interpolateConstant,
    FCurve::interpolateLinear,
    FCurve::interpolateBezier
};
