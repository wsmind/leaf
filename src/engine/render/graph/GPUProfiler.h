#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <sstream>

#include <d3d11.h>

class GPUProfiler
{
    public:
        void beginFrame();
        void endFrame();

        // returns an opaque handle on an internal structure
        int beginBlock(const std::string &name);
        void endBlock(int handle);
        
        // capture in the same JSON format as chrome://tracing
        // (allows analysis in the chrome tool directly)
        void beginJsonCapture();
        void endJsonCapture(const std::string filename); // actually writes the file

        class ScopedProfile
        {
            public:
                ScopedProfile(const std::string &name);
                ~ScopedProfile();

            private:
                int blockHandle;
        };
        
    private:
        GPUProfiler(bool enabled, ID3D11DeviceContext *context);
        ~GPUProfiler();

        ID3D11Query *requestPooledQuery();
        void releasePooledQuery(ID3D11Query *query);

        static GPUProfiler *instance;

        bool enabled;
        ID3D11DeviceContext *context;

        // queries are preallocated and used dynamically during frames
        // (only D3D11_QUERY_TIMESTAMP queries are pooled)
        static const unsigned int QUERY_POOL_SIZE = 2000;
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

        bool capturingJson;
        std::stringstream jsonData;

    public:
        // singleton implementation
        static void create(bool enabled, ID3D11DeviceContext *context) { assert(!GPUProfiler::instance); GPUProfiler::instance = new GPUProfiler(enabled, context); }
        static void destroy() { assert(GPUProfiler::instance); delete GPUProfiler::instance; }
        static GPUProfiler *getInstance() { assert(GPUProfiler::instance); return GPUProfiler::instance; }
};
