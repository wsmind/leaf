#include <engine/api.h>

int main()
{
    leaf_initialize(1280, 720, false);

    while (1)
    {
        leaf_render(1280, 720);
    }

    leaf_shutdown();

    return 0;
}
