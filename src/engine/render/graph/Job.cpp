#include <engine/render/graph/Job.h>

#include <engine/render/Device.h>

std::vector<unsigned char> Job::instanceBufferData;
ID3D11Buffer *Job::instanceBuffer = nullptr;
unsigned char *Job::instanceBufferPosition = nullptr;

Job::Job()
{
	this->instanceBufferOffset = (int)(Job::instanceBufferPosition - &Job::instanceBufferData[0]);
}

void Job::execute(ID3D11DeviceContext *context)
{
	ID3D11Buffer *buffers[] = { this->vertexBuffer, Job::instanceBuffer };
	UINT strides[] = { sizeof(float) * (3 /* pos */ + 3 /* normal */ + 4 /* tangent */ + 2 /* uv */), (UINT)this->instanceDataSize };
	UINT offsets[] = { 0, (UINT)this->instanceBufferOffset };
    context->IASetVertexBuffers(0, 2, buffers, strides, offsets);
    context->IASetIndexBuffer(this->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(this->indexCount, 0, 0);
}

void Job::createInstanceBuffer(int size)
{
	Job::instanceBufferData.resize(size);

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = size;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT res = Device::device->CreateBuffer(&bufferDesc, NULL, &Job::instanceBuffer);
	CHECK_HRESULT(res);

	Job::instanceBufferPosition = &Job::instanceBufferData[0];
}

void Job::destroyInstanceBuffer()
{
	Job::instanceBufferData.clear();
	Job::instanceBuffer->Release();
}

void Job::resetInstanceBufferPosition()
{
	Job::instanceBufferPosition = &Job::instanceBufferData[0];
}

void Job::applyInstanceBuffer()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT res = Device::context->Map(Job::instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CHECK_HRESULT(res);
	memcpy(mappedResource.pData, &Job::instanceBufferData[0], Job::instanceBufferData.size());
	Device::context->Unmap(Job::instanceBuffer, 0);
}
