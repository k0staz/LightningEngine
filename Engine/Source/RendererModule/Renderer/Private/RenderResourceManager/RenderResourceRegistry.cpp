#include "RenderResourceManager/RenderResourceRegistry.h"

#include "RenderContributors/RenderContributorManager.h"
#include "RenderContributors/MeshContributors/StaticMeshContributor.h"
#include "Service/ServiceRegistry.h"

namespace LE::Renderer
{
void StaticMeshRenderResourceStorage::ReleaseRenderResource(RenderResourceId ResourceId)
{
	if (!Has(ResourceId))
	{
		return;
	}

	ResourceState& state = ResourceStates.at(ResourceId);
	PopResource(ResourceId);
	ResourceMap.erase(state.AssetUid);
	ResourceStates.erase(ResourceId);
}

void StaticMeshRenderResourceStorage::ReleaseRenderResource(const Uid& AssetUid)
{
	auto it = ResourceMap.find(AssetUid);
	if (it != ResourceMap.end())
	{
		ReleaseRenderResource(ResourceStates[it->second].ResourceId);
	}
}

StaticMeshRenderResourceStorage::ResourceState StaticMeshRenderResourceStorage::GetCreateRenderResourceState(const Uid& AssetUid)
{
	auto it = ResourceMap.find(AssetUid);
	if (it != ResourceMap.end())
	{
		return ResourceStates[it->second];
	}
	
	StaticMeshRenderResource& meshRenderResource = static_cast<StaticMeshRenderResource&>(CreateNewResource());
	RenderContributorManager& contributorManager = GetServiceRegistry().GetService<RenderContributorManager>();
	StaticMeshContributor& renderContributor = contributorManager.CreateRenderContributor<StaticMeshContributor>();
	
	renderContributor.SetRenderResourceId(meshRenderResource);
	
	ResourceState newState;
	newState.AssetUid = AssetUid;
	newState.ResourceId = meshRenderResource.GetId();
	newState.ContributorInstanceId = renderContributor.GetInstanceId();
	
	ResourceMap[AssetUid] = newState.ResourceId;
	ResourceStates[newState.ResourceId] = newState;
	
	return newState;
}

StaticMeshRenderResourceStorage::ResourceState StaticMeshRenderResourceStorage::GetRenderResourceState(RenderResourceId ResourceId) const
{
	if (!Has(ResourceId))
	{
		return {};
	}
	
	return ResourceStates.at(ResourceId);
}

StaticMeshRenderResource& StaticMeshRenderResourceStorage::GetRenderResource(RenderResourceId ResourceId) const
{
	return static_cast<StaticMeshRenderResource&>(GetResource(ResourceId));
}

std::vector<StaticMeshRenderResourceStorage::ResourceState> StaticMeshRenderResourceStorage::GetAllRenderResourceStates() const
{
	std::vector<StaticMeshRenderResourceStorage::ResourceState> states;
	states.reserve(ResourceStates.size());

	for (const auto& it : ResourceStates)
	{
		states.push_back(it.second);
	}

	return states;
}
}
