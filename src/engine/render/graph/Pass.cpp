#include <engine/render/graph/Pass.h>

#include <engine/render/graph/GPUProfiler.h>

Pass::Pass(const std::string &name)
{
    this->name = name;
}

Pass::~Pass()
{
}

void Pass::execute(ID3D11DeviceContext *context)
{
    GPUProfiler::ScopedProfile profile(this->name);
}
