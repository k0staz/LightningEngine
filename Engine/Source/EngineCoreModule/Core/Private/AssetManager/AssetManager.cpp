#include "AssetManager/AssetManager.h"

namespace LE
{
void AssetManager::OnFrameEnd()
{
	if (PendingDelete.empty())
	{
		return;
	}

	LE_INFO("Removing assets:");

	for (auto& it : PendingDelete)
	{
		AssetStorageBase<IdType>& assetStorage = *AssetStorages[it.first];
		for (const AssetId& id : it.second)
		{
			assetStorage.PopResource(id);
			LE_INFO("	asset with id {} of type {} is removed", id, it.first);
		}
	}

	PendingDelete.clear();
}

void AssetManager::InternalLoadAsset(AssetInfo& Info) const
{
	const AssetStorageBase<IdType>& assetStorage = *GetAssetStorage(Info.TypeId);
	Asset& loadAsset = assetStorage.GetResource(Info.RuntimeId);
	AssetStorageFactory& factory = GetServiceRegistry().GetService<AssetStorageFactory>();
	const bool result = factory.Load(Info, loadAsset);
	loadAsset.SetState(result ? AssetState::Loaded : AssetState::FailedLoad);
}
}
