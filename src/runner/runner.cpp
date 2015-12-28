#include <engine/api.h>

int main()
{
    leaf_initialize();

    while (1)
    {
        leaf_render();
    }

    leaf_shutdown();

    return 0;
}
