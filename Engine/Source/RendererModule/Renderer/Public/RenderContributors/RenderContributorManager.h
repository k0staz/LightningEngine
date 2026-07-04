#pragma once
#include "RenderContributerCore.h"
#include "RenderContributor.h"
#include "RenderContributorStorage.h"
#include "Containers/ResourceSparseSet.h"
#include "Service/ServiceBase.h"

namespace LE::Renderer
{
class RenderContributorManager : public ServiceBase
{
	using RenderContributorBaseStorage = ResourceSparseSetBase<RenderContributorId, RenderContributor, RenderContributorTraits<RenderContributorId>>;
	
public:
	struct ContributorEntry
	{
		RenderContributorTypeId TypeId;
		RenderContributorId InstanceId;
	};

	void Initialize() override;
	void Shutdown() override;
	
	template<DerivedFromRenderContributor Type>
	Type& CreateRenderContributor(RenderContributorTypeId ContributorTypeId = RenderContributorTypeIdGetter<Type>::Value)
	{
		RenderContributorBaseStorage& contributorBaseStorage = GetCreateContributorStorage<Type>(ContributorTypeId);
		return static_cast<Type&>(contributorBaseStorage.CreateNewResource());
	}
	
	template<DerivedFromRenderContributor Type>
	Type& GetRenderContributor(RenderContributorId ContributorId) const
	{
		RenderContributorBaseStorage& contributorBaseStorage = GetContributorStorage(RenderContributorTypeIdGetter<Type>::Value);
		return static_cast<Type&>(contributorBaseStorage.GetResource(ContributorId));
	}
	
	RenderContributor& GetRenderContributor(RenderContributorTypeId ContributorTypeId, RenderContributorId ContributorId) const
	{
		RenderContributorBaseStorage& contributorBaseStorage = GetContributorStorage(ContributorTypeId);
		return contributorBaseStorage.GetResource(ContributorId);
	}
	
	void DeleteRenderContributor(RenderContributorTypeId ContributorTypeId, RenderContributorId ContributorId) const
	{
		RenderContributorBaseStorage& contributorBaseStorage = GetContributorStorage(ContributorTypeId);
		contributorBaseStorage.PopResource(ContributorId);
	}
	
	bool HasRenderContributor(RenderContributorTypeId ContributorTypeId, RenderContributorId ContributorId) const
	{
		if (!HasContributorStorage(ContributorTypeId))
		{
			return false;
		}
		
		RenderContributorBaseStorage& contributorBaseStorage = GetContributorStorage(ContributorTypeId);
		return contributorBaseStorage.Has(ContributorId);
	}
	
	void WriteContributorsFrameData(RefCountingPtr<RHI::RHILinearBuffer> FrameBuffer) const;

	[[nodiscard]] bool HasContributorAssetEntry(Uid AssetId) const
	{
		return ContributorAssetEntries.contains(AssetId);
	}

	[[nodiscard]] ContributorEntry GetContributorAssetEntry(Uid AssetId) const
	{
		return ContributorAssetEntries.at(AssetId);
	}

	template<DerivedFromRenderContributor Type>
	[[nodiscard]] Type& GetCreateContributorForAsset(Uid AssetId)
	{
		const RenderContributorTypeId typeId = RenderContributorTypeIdGetter<Type>::Value;
		if (!HasContributorAssetEntry(AssetId))
		{
			Type& contributor = CreateRenderContributor<Type>();
			ContributorAssetEntries.emplace(AssetId, ContributorEntry{ typeId, contributor.GetInstanceId() });
			return contributor;
		}

		ContributorEntry entry = GetContributorAssetEntry(AssetId);
		LE_ASSERT_DESC(typeId == entry.TypeId, "Requested Contributor Type doesn't match with the entry");
		return GetRenderContributor<Type>(entry.InstanceId);
	}
	
private:
	bool HasContributorStorage(RenderContributorTypeId ContributorTypeId) const
	{
		return ContributorStorages.contains(ContributorTypeId);
	}
	
	RenderContributorBaseStorage& GetContributorStorage(RenderContributorTypeId ContributorTypeId) const
	{
		return *ContributorStorages.at(ContributorTypeId);
	}
	
	template<DerivedFromRenderContributor Type>
	RenderContributorBaseStorage& GetCreateContributorStorage(RenderContributorTypeId ContributorTypeId = RenderContributorTypeIdGetter<Type>::Value)
	{
		auto it = ContributorStorages.find(ContributorTypeId);
		if (it == ContributorStorages.end())
		{
			auto storage = std::make_shared<RenderContributorStorage<Type>>();
			ContributorStorages.emplace(ContributorTypeId, storage);
			return *storage;
		}
		
		return *it->second;
	}
	
private:
	std::unordered_map<Uid, ContributorEntry> ContributorAssetEntries;
	std::unordered_map<RenderContributorTypeId, std::shared_ptr<RenderContributorBaseStorage>> ContributorStorages;
};
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::RenderContributorManager, "RenderContributorManager");
}
