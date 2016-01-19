#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

#include <Windows.h>

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

void *loadBlob(const std::string &filename)
{
    FILE *f = fopen(filename.c_str(), "rb");
    assert(f);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    void *buffer = malloc(size);
    fseek(f, 0, SEEK_SET);
    fread(buffer, size, 1, f);

    fclose(f);

    return buffer;
}

int main()
{
    leaf_initialize(1280, 720, false);

    std::vector<void *> blobs;
    WIN32_FIND_DATA fileInfo;
    HANDLE handle = FindFirstFile("*.bin", &fileInfo);
    if (handle != INVALID_HANDLE_VALUE)
    {
        while (true)
        {
            std::string filename = fileInfo.cFileName;
            std::string blobName = filename.substr(0, filename.length() - 4);

            void *buffer = loadBlob(filename);
            leaf_register_blob(blobName.c_str(), buffer);
            blobs.push_back(buffer);

            if (!FindNextFile(handle, &fileInfo))
                break;
        }
    }

    std::string data = loadFile("data.json");
    leaf_load_data(data.c_str());

    ShowCursor(FALSE);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        leaf_render(1280, 720);
    }

    ShowCursor(TRUE);

    leaf_shutdown();

    for (auto buffer: blobs)
    {
        free(buffer);
    }

    return 0;
}