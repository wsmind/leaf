#pragma once

#include <cassert>
#include <string>
#include <vector>

#include <d3d11.h>

class GPUProfiler
{
    public:
        void beginFrame();
        void endFrame();

        // returns an opaque handle on an internal structure
        int beginBlock(const std::string &name);
        void endBlock(int handle);
        
        class ScopedProfile
        {
            public:
                ScopedProfile(const std::string &name);
                ~ScopedProfile();

            private:
                int blockHandle;
        };
        
    private:
        GPUProfiler();
        ~GPUProfiler();

        ID3D11Query *requestPooledQuery();
        void releasePooledQuery(ID3D11Query *query);

        static GPUProfiler *instance;

        // queries are preallocated and used dynamically during frames
        // (only D3D11_QUERY_TIMESTAMP queries are pooled)
        static const unsigned int QUERY_POOL_SIZE = 100;
        std::vector<ID3D11Query *> queryPool;

        struct ProfilePoint
        {
            std::string name;
            ID3D11Query *startQuery;
            ID3D11Query *endQuery;
        };

        struct ProfileFrame
        {
            ID3D11Query *disjointQuery;
            std::vector<ProfilePoint> points;

            ProfileFrame()
                : disjointQuery(nullptr)
            {}
        };

        // this should be configured to make sure that the queries are always ready aftr this number of frames
        static const unsigned int FRAME_LATENCY = 8;
        ProfileFrame frames[FRAME_LATENCY];

        int currentFrameIndex;
        ProfileFrame *currentFrame;

        int frameBlock;

    public:
        // singleton implementation
        static void create() { assert(!GPUProfiler::instance); GPUProfiler::instance = new GPUProfiler; }
        static void destroy() { assert(GPUProfiler::instance); delete GPUProfiler::instance; }
        static GPUProfiler *getInstance() { assert(GPUProfiler::instance); return GPUProfiler::instance; }
};