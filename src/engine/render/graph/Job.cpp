#include <engine/render/graph/Job.h>

void Job::execute(ID3D11DeviceContext *context)
{
    UINT stride = sizeof(float) * (3 /* pos */ + 3 /* normal */ + 4 /* tangent */ + 2 /* uv */);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &this->vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(this->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    context->DrawIndexed(this->indexCount, 0, 0);
}
