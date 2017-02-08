#pragma once

template <typename ResourceType>
void Engine::loadDataCollection(cJSON *json, const std::string &collectionName)
{
    cJSON *elements = cJSON_GetObjectItem(json, collectionName.c_str());
    if (elements)
    {
        cJSON *element = elements->child;
        while (element)
        {
            std::string resourceName = element->string;
            ResourceManager::getInstance()->updateResourceData<ResourceType>(resourceName, element);

            element = element->next;
        }
    }
}
