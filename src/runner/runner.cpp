#define _CRT_SECURE_NO_WARNINGS
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

#include <Windows.h>

#include <engine/api.h>
#include <cJSON/cJSON.h>

void *loadFile(const std::string &filename, size_t *fileSize = nullptr)
{
    FILE *f = fopen(filename.c_str(), "rb");
    if (!f)
        return nullptr;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);

    if (fileSize != nullptr)
        *fileSize = size;

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

    float startFrame = 1.0f; // blender starts at frame 1
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

    char enginePath[MAX_PATH];
    GetModuleFileName(GetModuleHandle(nullptr), enginePath, MAX_PATH);

    std::string shaderPath = enginePath;
    shaderPath.resize(shaderPath.rfind('\\'));

    shaderPath += "/shaders/";

    leaf_initialize(width, height, false, profileFilename.empty() ? nullptr : profileFilename.c_str(), shaderPath.c_str());

    size_t dataSize;
    void *dataBuffer = loadFile("data.bin", &dataSize);
    assert(dataBuffer != nullptr);
    leaf_load_data(dataBuffer, dataSize);
    free(dataBuffer);

    void *audioBuffer = loadFile("music.wav");

    ShowCursor(FALSE);

    // render a few frames for delayed loadings and shader warmup
    for (int i = 0; i < 5; i++)
    {
        // get motion blur correct from the start
        leaf_update(startFrame - 1.0f);
        leaf_render(width, height, 1.0f / 60.0f);

        leaf_update(startFrame);
        leaf_render(width, height, 1.0f / 60.0f);
    }

    if (audioBuffer != nullptr)
        sndPlaySound((LPCSTR)audioBuffer, SND_ASYNC | SND_MEMORY);

    DWORD startTime = timeGetTime();
    DWORD previousTime = startTime;
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        DWORD currentTime = timeGetTime();
        float deltaTime = (float)(currentTime - previousTime) * 0.001f;
        previousTime = currentTime;

        float animationTime = (float)(currentTime - startTime) * 0.001f * fps + startFrame;
        leaf_update(animationTime);

        leaf_render(width, height, deltaTime);
    }

    ShowCursor(TRUE);

    leaf_shutdown();

    if (audioBuffer != nullptr)
        sndPlaySound(NULL, SND_ASYNC | SND_MEMORY);

    free(audioBuffer);

    return 0;
}
