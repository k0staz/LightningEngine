#pragma once

#include <vector>

#include "Asset.h"
#include "Containers/ResourceSparseSet.h"
#include "FileManager/FileManager.h"

namespace LE
{
template <Identifier IdType>
class AssetStorageBase : public ResourceSparseSetBase<IdType, Asset, AssetTraits<IdType>>
{
public:
	using base_type = ResourceSparseSetBase<IdType, Asset, AssetTraits<IdType>>;
	
	virtual Asset& CreateAsset(Uid StableId) = 0;
};

template <Identifier IdType, DerivedFromAsset AssetType>
class AssetStorage : public ResourceSparseSet<IdType, AssetType, Asset, AssetTraits<IdType>, AssetStorageBase<IdType>>
{
public:
	using base_type = ResourceSparseSet<IdType, AssetType, Asset, AssetTraits<IdType>>;
	Asset& CreateAsset(Uid StableId) override
	{
		IdType newAssetId = this->GetAvailableId();
		IdType& sparse = this->TryAdd(newAssetId);
		AssetType* asset = std::to_address(this->GetCreateResource(this->GetIndex(sparse)));
		std::construct_at(asset, newAssetId, StableId, AssetTypeIdGetter<AssetType>::Value);
		return static_cast<Asset&>(*asset);
	}
};
}
