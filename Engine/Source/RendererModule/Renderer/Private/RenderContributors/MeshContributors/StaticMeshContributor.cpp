#include "RenderContributors/MeshContributors/StaticMeshContributor.h"

#include "Service/ServiceRegistry.h"
#include "RenderResourceManager/RenderResourceManager.h"
#include "Templates/Alignment.h"

namespace LE::Renderer
{
StaticMeshContributor::~StaticMeshContributor()
{
	ProxyDynamicDataStorage.Clear();
	ThisFrameDynamicData.clear();
}

StaticMeshContributor::StaticMeshDynamicData& StaticMeshContributor::GetDynamicData(EcsEntity Entity)
{
	if (ProxyDynamicDataStorage.Has(Entity))
	{
		return ProxyDynamicDataStorage.GetInstance(Entity);
	}
	
	static StaticMeshContributor::StaticMeshDynamicData fallbackData;
	LE_ASSERT_DESC(false, "StaticMeshContributor::GetDynamicData: Entity not found")
	return fallbackData;
}

void StaticMeshContributor::SetRenderResourceId(RenderResourceHandle<const StaticMeshRenderResource> Resource)
{
	RenderResource = Resource;

}

bool StaticMeshContributor::HasProxy(EcsEntity Entity)
{
	return ProxyDynamicDataStorage.Has(Entity);
}

void StaticMeshContributor::AddProxy(EcsEntity Entity)
{
	ProxyDynamicDataStorage.CreateInstance(Entity);
}

void StaticMeshContributor::RemoveProxy(EcsEntity Entity)
{
	if (ProxyDynamicDataStorage.Has(Entity))
	{
		ProxyDynamicDataStorage.Delete(Entity);
	}
}

void StaticMeshContributor::WriteFrameDataDynamicResources(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer)
{
	if (!RenderResource.IsValid())
	{
		return;
	}

	const StaticMeshRenderResource& staticMeshRenderResource = RenderResource.Get();

	if (!staticMeshRenderResource.IsLoaded())
	{
		ThisFrameDynamicData.clear();
		return;
	}
	
	StaticMeshFrameData thisFrameData = {};

	StaticMeshResources staticMeshResources = {};
	staticMeshResources.IndicesFetchPtr = staticMeshRenderResource.IndexBuffer->GetGpuAddress();
	staticMeshResources.PositionsFetchPtr = staticMeshRenderResource.PositionBuffer->GetGpuAddress();
	staticMeshResources.NormalsFetchPtr = staticMeshRenderResource.NormalBuffer->GetGpuAddress();
	staticMeshResources.TexCoordsFetchPtr = staticMeshRenderResource.TexCoordsBuffer->GetGpuAddress();
	
	thisFrameData.ResourceDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
	FrameBuffer->Write(&staticMeshResources, sizeof(StaticMeshResources));
	
	thisFrameData.DynamicDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
	for (auto& it : ThisFrameDynamicData)
	{
		FrameBuffer->Write(it, sizeof(StaticMeshDynamicData));
	}
	
	ThisFrameDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
	FrameBuffer->Write(&thisFrameData, sizeof(StaticMeshFrameData));
	
	ThisFrameDynamicData.clear();
}

void StaticMeshContributor::AddProxyToThisFrameContribution(EcsEntity Entity)
{
	if (!ProxyDynamicDataStorage.Has(Entity))
	{
		return;
	}

	auto& dynamicData = ProxyDynamicDataStorage.GetInstance(Entity);
	ThisFrameDynamicData.push_back(&dynamicData);
}

uint32 StaticMeshContributor::GetIndexCount() const
{
	if (!RenderResource.IsValid())
	{
		return 0;
	}

	const StaticMeshRenderResource& staticMeshRenderResource = RenderResource.Get();
	if (!staticMeshRenderResource.IsLoaded())
	{
		return 0;
	}

	return staticMeshRenderResource.IndexCount;
}

bool StaticMeshContributor::IsReady() const
{
	return IsStaticMeshResourceReady();
}

bool StaticMeshContributor::IsStaticMeshResourceReady() const
{
	if (!RenderResource.IsValid())
	{
		return false;
	}

	return RenderResource.Get().IsLoaded();
}
}
