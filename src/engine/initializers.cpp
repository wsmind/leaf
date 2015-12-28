#include <windows.h>

#include <engine/Engine.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            Engine::create();
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            Engine::destroy();
            break;
        }
    }
    return TRUE;
}
