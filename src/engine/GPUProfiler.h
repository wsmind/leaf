#pragma once

#include <cassert>
#include <string>

class GPUProfiler
{
    public:
        static void beginFrame();
        static void endFrame();
        
        class ScopedProfile
        {
            public:
                ScopedProfile(const std::string &name);
                ~ScopedProfile();
        };
        
    private:
};
