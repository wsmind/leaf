#include <engine/Action.h>

#include <engine/ResourceManager.h>

const std::string Action::resourceClassName = "Action";
const std::string Action::defaultResourceData = "{\"fcurves\": {}}";

void Action::load(const cJSON *json)
{
    cJSON *fcurves = cJSON_GetObjectItem(json, "fcurves");
    printf("Action: loading %d fcurves\n", cJSON_GetArraySize(fcurves));
}

void Action::unload()
{
}
