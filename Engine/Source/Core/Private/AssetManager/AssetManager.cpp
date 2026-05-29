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
			assetStorage.PopAsset(id);
			LE_INFO("	asset with id {} of type {} is removed", id, it.first);
		}
	}

	PendingDelete.clear();
}
}
