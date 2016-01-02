#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <string>

#include <engine/api.h>
#include <engine/cJSON/cJSON.h>

std::string loadFile(const std::string &filename)
{
    FILE *f = fopen(filename.c_str(), "rb");
    assert(f);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    void *buffer = malloc(size);
    fseek(f, 0, SEEK_SET);
    fread(buffer, size, 1, f);

    std::string str((const char *)buffer, size);

    free(buffer);
    fclose(f);

    return str;
}

int main()
{
    leaf_initialize(1280, 720, false);

    std::string data = loadFile("data.json");
    leaf_load_data(data.c_str());

    while (1)
    {
        leaf_render(1280, 720);
    }

    leaf_shutdown();

    return 0;
}
