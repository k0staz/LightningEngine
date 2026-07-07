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

void StaticMeshContributor::SetRenderResource(RenderResourceHandle<const StaticMeshRenderResource> Resource)
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
	ThisFrameGpuAddress.clear();
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

	StaticMeshResources staticMeshResources = {};
	staticMeshResources.IndicesFetchPtr = staticMeshRenderResource.IndexBuffer->GetGpuAddress();
	staticMeshResources.PositionsFetchPtr = staticMeshRenderResource.PositionBuffer->GetGpuAddress();
	staticMeshResources.NormalsFetchPtr = staticMeshRenderResource.NormalBuffer->GetGpuAddress();
	staticMeshResources.TexCoordsFetchPtr = staticMeshRenderResource.TexCoordsBuffer->GetGpuAddress();

	uint64 resourceGpuData = FrameBuffer->GetCurrentGpuAddress();
	FrameBuffer->Write(&staticMeshResources, sizeof(StaticMeshResources));

	for (auto& it : ThisFrameDynamicData)
	{
		StaticMeshFrameData thisFrameData = {};
		thisFrameData.ResourceDataGpuAddress = resourceGpuData;
		thisFrameData.DynamicDataGpuAddress = FrameBuffer->GetCurrentGpuAddress();
		for (auto& dynamicData : it.second)
		{
			FrameBuffer->Write(dynamicData, sizeof(StaticMeshDynamicData));
		}

		ThisFrameGpuAddress[it.first] = FrameBuffer->GetCurrentGpuAddress();
		FrameBuffer->Write(&thisFrameData, sizeof(StaticMeshFrameData));
	}

	ThisFrameDynamicData.clear();
}

void StaticMeshContributor::AddProxyToThisFrameContribution(EcsEntity Entity, PermutationVariationKey BatchKey)
{
	if (!ProxyDynamicDataStorage.Has(Entity))
	{
		return;
	}

	auto& dynamicData = ProxyDynamicDataStorage.GetInstance(Entity);
	ThisFrameDynamicData[BatchKey].push_back(&dynamicData);
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

uint64 StaticMeshContributor::GetThisFrameDataGPUAddress(PermutationVariationKey BatchKey) const
{
	if (!ThisFrameGpuAddress.contains(BatchKey))
	{
		return 0;
	}
	return ThisFrameGpuAddress.at(BatchKey);
}
}
