#pragma once

#include <cassert>
#include <vector>

#include <d3d11.h>

class Job
{
    public:
		Job::Job();

        void setBuffers(ID3D11Buffer *vertexBuffer, ID3D11Buffer *indexBuffer, int indexCount)
        {
            this->vertexBuffer = vertexBuffer;
            this->indexBuffer = indexBuffer;
            this->indexCount = indexCount;
        }

		template <typename InstanceData>
		void addInstance(const InstanceData &instanceData);

		void addInstance() { assert(this->instanceDataSize == 0);  this->instanceCount++; }

		void addDispatch(int x, int y, int z)
		{
			this->dispatchSizeX = x;
			this->dispatchSizeY = y;
			this->dispatchSizeZ = z;
		}

        void execute(ID3D11DeviceContext *context);
		
		static void createInstanceBuffer(int size);
		static void destroyInstanceBuffer();

		static void resetInstanceBufferPosition();
		static void applyInstanceBuffer();

    private:
		static std::vector<unsigned char> instanceBufferData;
		static ID3D11Buffer *instanceBuffer;
		static unsigned char *instanceBufferPosition;

		ID3D11Buffer *vertexBuffer = nullptr;
        ID3D11Buffer *indexBuffer = nullptr;
        int indexCount = 0;
		int instanceCount = 0;
		int instanceBufferOffset = 0;
		int instanceBufferSlice = 0;
		int instanceDataSize = 0;

		int dispatchSizeX = 0;
		int dispatchSizeY = 0;
		int dispatchSizeZ = 0;
};

template <typename InstanceData>
void Job::addInstance(const InstanceData &instanceData)
{
	assert((this->instanceCount == 0) || (this->instanceDataSize == sizeof(InstanceData)));

	if (Job::instanceBufferPosition + sizeof(InstanceData) > &Job::instanceBufferData[0] + Job::instanceBufferData.size())
	{
		printf("Too many instances for this frame\n");
		return;
	}

	memcpy(Job::instanceBufferPosition, &instanceData, sizeof(InstanceData));
	this->instanceCount++;

	this->instanceDataSize = sizeof(InstanceData);
	this->instanceBufferSlice += this->instanceDataSize;
	Job::instanceBufferPosition += this->instanceDataSize;
}
