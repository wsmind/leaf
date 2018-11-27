#include "scene.h"
#include "pass.h"
#include "postprocess.h"

POSTPROCESS_PS_INPUT main(POSTPROCESS_VS_INPUT input)
{
    POSTPROCESS_PS_INPUT output;
    output.pos = float4(input.pos.xy, 0.0, 1.0);
    output.uv = input.uv;

    return output;
}
