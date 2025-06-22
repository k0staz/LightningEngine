#include "RenderResource.h"

#include "RenderCommandList.h"

namespace LE::Renderer
{
RenderResource::~RenderResource()
{
    ReleaseRHI();
}

RefCountingPtr<RHI::RHIBuffer> RenderResource::CreateRHIBuffer(ResourceArrayInterface* ResourceData,
                                                               RHI::BufferUsageFlags BufferUsage)
{
    const uint32 dataSizeInBytes = ResourceData ? ResourceData->GetResourceDataSize() : 0;
    RHI::RHIResourceCreateInfo createInfo(ResourceData);

	return RenderCommandList::Get().CreateBuffer(dataSizeInBytes, BufferUsage, 0, createInfo);
}
}
