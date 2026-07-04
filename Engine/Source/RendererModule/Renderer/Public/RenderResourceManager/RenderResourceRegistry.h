#pragma once
#include "RenderResources.h"
#include "Containers/ResourceSparseSet.h"
#include "RenderContributors/RenderContributerCore.h"

namespace LE::Renderer
{
class RenderResourceStorageBase : public ResourceSparseSetBase<RenderResourceId, RenderResource, RenderResourceTraits<RenderResourceId>>
{
public:
	using base_type = ResourceSparseSetBase<RenderResourceId, RenderResource, RenderResourceTraits<RenderResourceId>>;

	struct ResourceState
	{
		[[nodiscard]] bool IsValid() const
		{
			return ResourceId != NullId{};
		}

		Uid AssetUid;
		RenderResourceId ResourceId = NullId{};
	};

	void ReleaseRenderResource(RenderResourceId ResourceId)
	{
		if (!this->Has(ResourceId))
		{
			return;
		}

		ResourceState& state = ResourceStates.at(ResourceId);
		base_type::PopResource(ResourceId);
		ResourceMap.erase(state.AssetUid);
		ResourceStates.erase(ResourceId);
	}

	void ReleaseRenderResource(const Uid& AssetUid)
	{
		auto it = ResourceMap.find(AssetUid);
		if (it != ResourceMap.end())
		{
			ReleaseRenderResource(ResourceStates[it->second].ResourceId);
		}
	}

	ResourceState GetCreateRenderResourceState(const Uid& AssetUid)
	{
		auto it = ResourceMap.find(AssetUid);
		if (it != ResourceMap.end())
		{
			return ResourceStates[it->second];
		}

		RenderResource& renderResource = CreateNewResource();

		ResourceState newState;
		newState.AssetUid = AssetUid;
		newState.ResourceId = renderResource.GetId();

		ResourceMap[AssetUid] = newState.ResourceId;
		ResourceStates[newState.ResourceId] = newState;

		return newState;
	}

	[[nodiscard]] ResourceState GetRenderResourceState(RenderResourceId ResourceId) const
	{
		if (!this->Has(ResourceId))
		{
			return {};
		}

		return ResourceStates.at(ResourceId);
	}

	[[nodiscard]] std::vector<ResourceState> GetAllRenderResourceStates() const
	{
		std::vector<ResourceState> states;
		states.reserve(ResourceStates.size());

		for (const auto& it : ResourceStates)
		{
			states.push_back(it.second);
		}

		return states;
	}

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

template<DerivedFromRenderResource RenderResourceType>
class RenderResourceStorage final : public ResourceSparseSet<RenderResourceId, RenderResourceType, RenderResource, RenderResourceTraits<RenderResourceId>, RenderResourceStorageBase>
{
public:
	RenderResource& CreateNewResource() override
	{
		RenderContributorId contributorId = this->GetAvailableId();
		RenderContributorId& sparse = this->TryAdd(contributorId);
		RenderResourceType* contributor = std::to_address(this->GetCreateResource(this->GetIndex(sparse)));
		std::construct_at(contributor, contributorId, RenderResourceTypeIdGetter<RenderResourceType>::Value);
		return *contributor;
	}
};
}
