#include <engine/Action.h>

#include <engine/FCurve.h>
#include <engine/PropertyMapping.h>
#include <engine/ResourceManager.h>

#include <engine/cJSON/cJSON.h>

const std::string Action::resourceClassName = "Action";
const std::string Action::defaultResourceData = "{\"fcurves\": {}}";

void Action::load(const unsigned char *buffer, size_t size)
{
    cJSON *json = cJSON_Parse((const char *)buffer);

    cJSON *fcurves = cJSON_GetObjectItem(json, "fcurves");

    cJSON *fcurveData = fcurves->child;
    while (fcurveData)
    {
        this->curves.push_back(new FCurve(fcurveData));
        fcurveData = fcurveData->next;
    }

    cJSON_Delete(json);
}

void Action::unload()
{
    for (auto curve: this->curves)
    {
        delete curve;
    }
    this->curves.clear();
}

void Action::evaluate(float time, const PropertyMapping *properties) const
{
    for (auto curve: this->curves)
    {
        curve->evaluate(time, properties);
    }
}
