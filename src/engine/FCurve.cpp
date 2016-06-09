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
        key.co.x = (float)cJSON_GetArrayItem(keyframeData, 0)->valuedouble;
        key.co.y = (float)cJSON_GetArrayItem(keyframeData, 1)->valuedouble;
        this->keyframes.push_back(key);

        keyframeData = keyframeData->next;
    }
}

void FCurve::evaluate(float time, const PropertyMapping *properties)
{
    float *property = properties->get(this->path, this->index);
    if (!property)
        return;
    
    int i = 1;
    while ((i < this->keyframes.size() - 1) && (time >= this->keyframes[i].co.x))
        i++;

    i--;
    if (i < this->keyframes.size() - 1)
    {
        // basic lerp
        float t = (time - this->keyframes[i].co.x) / (this->keyframes[i + 1].co.x - this->keyframes[i].co.x);
        *property = this->keyframes[i].co.y + (this->keyframes[i + 1].co.y - this->keyframes[i].co.y) * t;
    }
    else
    {
        *property = this->keyframes[i].co.y;
    }
}
