#include <engine/render/graph/Pass.h>

#include <engine/render/Device.h>
#include <engine/render/graph/Batch.h>
#include <engine/render/graph/GPUProfiler.h>

Pass::Pass(const std::string &name)
    : name(name)
{
    D3D11_VIEWPORT defaultViewport;
    defaultViewport.Width = 16.0f;
    defaultViewport.Height = 16.0f;
    defaultViewport.MinDepth = 0.0f;
    defaultViewport.MaxDepth = 1.0f;
    defaultViewport.TopLeftX = 0;
    defaultViewport.TopLeftY = 0;

    this->setViewport(defaultViewport, glm::mat4(1.0f), glm::mat4(1.0f));
}

void Pass::setViewport(D3D11_VIEWPORT viewport, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
	assert(viewport.Width > 0.0f);
	assert(viewport.Height > 0.0f);

	this->viewport = viewport;

    // derive secondary info from matrices
    glm::mat4 viewMatrixInverse = glm::inverse(viewMatrix);
    this->passConstants.viewMatrix = viewMatrix;
    this->passConstants.viewMatrixInverse = viewMatrixInverse;
    this->passConstants.projectionMatrix = projectionMatrix;
    this->passConstants.projectionMatrixInverse = glm::inverse(projectionMatrix);
    this->passConstants.viewProjectionInverseMatrix = glm::inverse(projectionMatrix * viewMatrix);
    this->passConstants.cameraPosition = glm::vec3(viewMatrixInverse[3][0], viewMatrixInverse[3][1], viewMatrixInverse[3][2]);
	this->passConstants.viewportSize = glm::vec4(viewport.Width, viewport.Height, 1.0f / viewport.Width, 1.0f / viewport.Height);
}

void Pass::setViewport(float width, float height, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix)
{
	// user defaults for other viewport parameters
	D3D11_VIEWPORT viewport;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	this->setViewport(viewport, viewMatrix, projectionMatrix);
}

Batch *Pass::addBatch(const std::string &name)
{
    Batch *batch = new Batch(name);
    this->batches.push_back(batch);
    return batch;
}

void Pass::execute(ID3D11DeviceContext *context, ID3D11Buffer *passConstantBuffer, ID3DUserDefinedAnnotation *annotation)
{
    GPUProfiler::ScopedProfile profile(this->name);

	std::wstring nameWide(name.begin(), name.end());
	annotation->BeginEvent(nameWide.c_str());

    // upload pass constants to GPU
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT res = context->Map(passConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    CHECK_HRESULT(res);
    memcpy(mappedResource.pData, &this->passConstants, sizeof(PassConstants));
    context->Unmap(passConstantBuffer, 0);

    context->RSSetViewports(1, &this->viewport);

    if ((this->colorTargets.size() > 0) || (this->depthStencilTarget != nullptr))
        context->OMSetRenderTargets((UINT)this->colorTargets.size(), this->colorTargets.data(), this->depthStencilTarget);

    // render batches
    for (auto *batch : this->batches)
    {
        batch->execute(context);
        delete batch;
    }

    if ((this->colorTargets.size() > 0) || (this->depthStencilTarget != nullptr))
        context->OMSetRenderTargets(0, nullptr, nullptr);

	annotation->EndEvent();
}
