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
    if (!f)
        return nullptr;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);

    void *buffer = malloc(size);
    fseek(f, 0, SEEK_SET);
    fread(buffer, size, 1, f);

    fclose(f);

    return buffer;
}

int main(int argc, char **argv)
{
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    float fps = 60.0f; // hardcoded 60fps

    float startFrame = 0;
    std::string profileFilename;

    int argIndex = 1;
    while (argIndex < argc)
    {
        std::string arg(argv[argIndex]);

        size_t pos = arg.find('=');
        if (pos != std::string::npos)
        {
            auto key = arg.substr(0, pos);
            auto value = arg.substr(pos + 1);

            if (key == "--start-frame")
            {
                startFrame = (float)atof(value.c_str());
                printf("start frame: %f\n", startFrame);
            }
            else if (key == "--profile")
            {
                profileFilename = value;
            }
        }

        argIndex++;
    }

    leaf_initialize(width, height, false, profileFilename.empty() ? nullptr : profileFilename.c_str());

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
            assert(buffer != nullptr);
            leaf_register_blob(blobName.c_str(), buffer);
            blobs.push_back(buffer);

            if (!FindNextFile(handle, &fileInfo))
                break;
        }
    }

    void *audioBuffer = loadBlob("music.wav");

    std::string data = loadFile("data.json");
    leaf_load_data(data.c_str());

    ShowCursor(FALSE);

    if (audioBuffer != nullptr)
        sndPlaySound((LPCSTR)audioBuffer, SND_ASYNC | SND_MEMORY);

    DWORD startTime = timeGetTime();
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        float time = (float)(timeGetTime() - startTime) * 0.001f * fps + startFrame;
        leaf_update_animation(time);
        leaf_render(width, height);
    }

    ShowCursor(TRUE);

    leaf_shutdown();

    if (audioBuffer != nullptr)
        sndPlaySound(NULL, SND_ASYNC | SND_MEMORY);

    free(audioBuffer);

    for (auto buffer: blobs)
    {
        free(buffer);
    }

    return 0;
}
