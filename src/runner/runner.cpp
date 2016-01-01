#include <engine/api.h>

int main()
{
    leaf_initialize(1280, 720, false);

    leaf_load_data("{ \"objects\": {}, \"materials\" : {\"Material\": {\"diffuse\": [0.800000011920929, 0.800000011920929, 0.800000011920929]}} }");

    while (1)
    {
        leaf_render(1280, 720);
    }

    leaf_shutdown();

    return 0;
}
