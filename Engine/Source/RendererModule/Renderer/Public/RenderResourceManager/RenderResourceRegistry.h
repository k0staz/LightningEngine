#pragma once
#include "RenderResources.h"
#include "Containers/ResourceSparseSet.h"
#include "RenderContributors/RenderContributerCore.h"

namespace LE::Renderer
{
class StaticMeshRenderResourceStorage final : public ResourceSparseSet<RenderResourceId, StaticMeshRenderResource, RenderResource, RenderResourceTraits<RenderResourceId>>
{
public:
	struct ResourceState
	{
		bool IsValid() const
		{
			return ResourceId != NullId{};
		}

		Uid AssetUid;
		RenderResourceId ResourceId = NullId{};
		RenderContributorId ContributorInstanceId = NullId{};
	};
	
	StaticMeshRenderResource& CreateNewResource() override
	{
		RenderContributorId contributorId = GetAvailableId();
		RenderContributorId& sparse = TryAdd(contributorId);
		StaticMeshRenderResource* contributor = std::to_address(GetCreateResource(base_type::GetIndex(sparse)));
		std::construct_at(contributor, contributorId, RenderResourceTypeIdGetter<StaticMeshRenderResource>::Value);
		return static_cast<StaticMeshRenderResource&>(*contributor);
	}

	void ReleaseRenderResource(RenderResourceId ResourceId);
	void ReleaseRenderResource(const Uid& AssetUid);
	
	ResourceState GetCreateRenderResourceState(const Uid& AssetUid);
	ResourceState GetRenderResourceState(RenderResourceId ResourceId) const;
	
	StaticMeshRenderResource& GetRenderResource(RenderResourceId ResourceId) const;

	std::vector<ResourceState> GetAllRenderResourceStates() const;

	void Clear() override
	{
		base_type::Clear();
		ResourceMap.clear();
		ResourceStates.clear();
	}
private:
	std::unordered_map<Uid, RenderResourceId> ResourceMap;
	std::unordered_map<RenderResourceId, ResourceState> ResourceStates;
};
}
