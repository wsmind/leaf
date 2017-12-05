#include <engine/render/graph/GPUProfiler.h>

#include <fstream>

#include <engine/render/Device.h>

GPUProfiler *GPUProfiler::instance = nullptr;

void GPUProfiler::beginFrame()
{
    if (!this->enabled)
        return;

    // for the first few frames (as configured in FRAME_LATENCY), new disjoint will be created
    // (they will be reused for all the subsequent frames)
    if (!this->currentFrame->disjointQuery)
    {
        D3D11_QUERY_DESC queryDesc;
        ZeroMemory(&queryDesc, sizeof(queryDesc));
        queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

        Device::device->CreateQuery(&queryDesc, &this->currentFrame->disjointQuery);
    }

    // start the enclosing disjoint query
    Device::context->Begin(this->currentFrame->disjointQuery);

    // automatic frame block
    this->frameBlock = this->beginBlock("Frame");
}

void GPUProfiler::endFrame()
{
    if (!this->enabled)
        return;

    // automatic frame block
    this->endBlock(this->frameBlock);

    // finish the frame disjoint query
    // don't retrieve anything right now, the results should be ready in FRAME_LATENCY
    Device::context->End(this->currentFrame->disjointQuery);

    // switch to next frame (should contain results from FRAME_LATENCY frames ago, and will
    // be replaced by the next profiled frame)
    this->currentFrameIndex = (this->currentFrameIndex + 1) % FRAME_LATENCY;
    this->currentFrame = &this->frames[this->currentFrameIndex];

    // skip if this frame was never used
    if (!this->currentFrame->disjointQuery)
        return;

    // retrieve results
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT queryDataDisjoint;
    HRESULT result = Device::context->GetData(this->currentFrame->disjointQuery, &queryDataDisjoint, sizeof(queryDataDisjoint), 0);

    // if data is still not available, FRAME_LATENCY should be increased
    assert(result == S_OK);

    // collect measurements if this frame is not disjoint
    if (queryDataDisjoint.Disjoint == FALSE)
    {
        for (auto &point: this->currentFrame->points)
        {
            UINT64 start;
            UINT64 end;

            Device::context->GetData(point.startQuery, &start, sizeof(UINT64), 0);
            Device::context->GetData(point.endQuery, &end, sizeof(UINT64), 0);

            // conversion to microseconds
            UINT64 startUs = start * 1000000 / queryDataDisjoint.Frequency;
            UINT64 endUs = end * 1000000 / queryDataDisjoint.Frequency;
            UINT64 durationUs = endUs - startUs;

            if (this->capturingJson)
                this->jsonData << "{\"pid\":\"Leaf\",\"tid\":\"GPU\",\"ts\":" << startUs << ",\"ph\":\"X\",\"cat\":\"gpu\",\"name\":\"" << point.name << "\",\"dur\":" << durationUs << "}," << std::endl;
        }
    }

    // return all queries to the pool
    for (auto &point: this->currentFrame->points)
    {
        this->releasePooledQuery(point.startQuery);
        this->releasePooledQuery(point.endQuery);
    }
    this->currentFrame->points.clear();
}

int GPUProfiler::beginBlock(const std::string &name)
{
    if (!this->enabled)
        return 0;

    ProfilePoint point;
    point.name = name;
    point.startQuery = this->requestPooledQuery();
    point.endQuery = this->requestPooledQuery();

    // record the start timestamp
    Device::context->End(point.startQuery);

    this->currentFrame->points.push_back(point);
    return (int)this->currentFrame->points.size() - 1;
}

void GPUProfiler::endBlock(int handle)
{
    if (!this->enabled)
        return;

    ProfilePoint &point = this->currentFrame->points[handle];

    // record the end timestamp
    Device::context->End(point.endQuery);
}

void GPUProfiler::beginJsonCapture()
{
    if (!this->enabled)
        return;

    assert(this->capturingJson == false);

    this->capturingJson = true;

    // capture prefix
    this->jsonData << "[" << std::endl;
}

void GPUProfiler::endJsonCapture(const std::string filename)
{
    if (!this->enabled)
        return;

    assert(this->capturingJson == true);

    // dump content to file
    std::ofstream f(filename.c_str());
    assert(f.good());
    std::string text = this->jsonData.str();
    f.write(text.c_str(), text.size());

    this->capturingJson = false;
    this->jsonData.clear();
}

GPUProfiler::ScopedProfile::ScopedProfile(const std::string &name)
{
    this->blockHandle = GPUProfiler::getInstance()->beginBlock(name);
}

GPUProfiler::ScopedProfile::~ScopedProfile()
{
    GPUProfiler::getInstance()->endBlock(this->blockHandle);
}

GPUProfiler::GPUProfiler(bool enabled)
{
    // when the profiler is disabled, every call is stubbed to do nothing
    this->enabled = enabled;

    if (!this->enabled)
        return;

    this->currentFrameIndex = 0;
    this->currentFrame = &this->frames[this->currentFrameIndex];

    this->frameBlock = 0;

    this->capturingJson = false;

    // build the query pool
    D3D11_QUERY_DESC queryDesc;
    ZeroMemory(&queryDesc, sizeof(queryDesc));
    queryDesc.Query = D3D11_QUERY_TIMESTAMP;

    this->queryPool.reserve(QUERY_POOL_SIZE);
    for (unsigned int i = 0; i < QUERY_POOL_SIZE; i++)
    {
        ID3D11Query *query;
        Device::device->CreateQuery(&queryDesc, &query);

        this->queryPool.push_back(query);
    }
}

GPUProfiler::~GPUProfiler()
{
    if (!this->enabled)
        return;

    // destroy frame data
    for (int i = 0; i < FRAME_LATENCY; i++)
    {
        ProfileFrame &frame = this->frames[i];

        // check if this object has ever been used
        if (!frame.disjointQuery)
            continue;
        
        // destroy the frame-wide disjoint query
        frame.disjointQuery->Release();

        // release all the queries currently in flight
        for (auto &point: frame.points)
        {
            this->releasePooledQuery(point.startQuery);
            this->releasePooledQuery(point.endQuery);
        }
        frame.points.clear();
    }

    // ensure all queries in flight have been returned to the pool
    assert(this->queryPool.size() == QUERY_POOL_SIZE);

    // destroy the query pool
    for (unsigned int i = 0; i < QUERY_POOL_SIZE; i++)
    {
        ID3D11Query *query = this->queryPool[i];
        query->Release();
    }
}

ID3D11Query *GPUProfiler::requestPooledQuery()
{
    assert(this->queryPool.size() >= 1);

    ID3D11Query *query = this->queryPool.back();
    this->queryPool.pop_back();

    return query;
}

void GPUProfiler::releasePooledQuery(ID3D11Query *query)
{
    this->queryPool.push_back(query);

    // check for too many release calls
    assert(this->queryPool.size() <= QUERY_POOL_SIZE);
}
