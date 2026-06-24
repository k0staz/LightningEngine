#include "AssetManager/AssetRegistry.h"

#include "FileManager/FileManager.h"

namespace LE
{
// TODO: These should be stored in the config
#define MANIFEST_MAP_PATH "AssetManifest.leasset"

void AssetRegistry::Initialize()
{
	LoadManifest();
}

void AssetRegistry::Shutdown()
{
}

Uid AssetRegistry::GetUidFromPath(const Path& AssetPath) const
{
	std::shared_lock lock(assetInfoMutex);
	if (PathToUid.contains(AssetPath))
	{
		return PathToUid.at(AssetPath);
	}

	return {};
}

bool AssetRegistry::IsValidAsset(const Uid& AssetUid) const
{
	std::shared_lock lock(assetInfoMutex);
	return AssetUid.IsValid() && UidToAssetInfo.contains(AssetUid);
}

AssetInfo AssetRegistry::GetAssetInfo(const Uid& AssetUid) const
{
	std::shared_lock lock(assetInfoMutex);
	if (IsValidAsset(AssetUid))
	{
		return UidToAssetInfo.at(AssetUid);
	}

	return {};
}

void AssetRegistry::SetAssetInfo(const AssetInfo& Info)
{
	std::unique_lock lock(assetInfoMutex);
	UidToAssetInfo[Info.AssetUid] = Info;
}

void AssetRegistry::RemoveAssetFromRegistry(const Uid& AssetUid)
{
	if(!IsValidAsset(AssetUid))
	{
		return;
	}
	std::unique_lock lock(assetInfoMutex);
	PathToUid.erase(UidToAssetInfo[AssetUid].PathToAsset);
	UidToAssetInfo.erase(AssetUid);
}

void AssetRegistry::RemoveAssetFromRegistry(const Path& AssetPath)
{
	RemoveAssetFromRegistry(GetUidFromPath(AssetPath));
}

void AssetRegistry::UpdateAssetRuntimeId(const Uid& AssetUid, AssetId RuntimeId)
{
	std::unique_lock lock(assetInfoMutex);
	if (!UidToAssetInfo.contains(AssetUid))
	{
		LE_WARN("There is not asset info for the {} UID", Uid::ToString(AssetUid));
	}

	UidToAssetInfo[AssetUid].RuntimeId = RuntimeId;
}

void AssetRegistry::UpdateAssetPath(const Uid& AssetUid, const Path& AssetPath)
{
	if (!IsValidAsset(AssetUid))
	{
		return;
	}

	std::unique_lock lock(assetInfoMutex);
	AssetInfo& info = UidToAssetInfo[AssetUid];
	PathToUid.erase(info.PathToAsset);

	info.PathToAsset = AssetPath;
	PathToUid[AssetPath] = AssetUid;
}

void AssetRegistry::AddLoadingTask(const Uid& AssetUid, RefCountingPtr<AsyncTaskNodeBase> LoadingTask)
{
	if (!IsValidAsset(AssetUid))
	{
		LE_ASSERT_DESC(false, "Trying to add a loading task to an invalid asset with UID {}", Uid::ToString(AssetUid).c_str())
		return;
	}

	std::unique_lock lock(assetInfoMutex);
	AssetInfo& info = UidToAssetInfo[AssetUid];

	info.LoadingTask = LoadingTask;
}

void AssetRegistry::RemoveLoadingTask(const Uid& AssetUid)
{
	if (!IsValidAsset(AssetUid))
	{
		LE_ASSERT_DESC(false, "Trying to remove a loading task from an invalid asset with UID {}", Uid::ToString(AssetUid).c_str())
			return;
	}
	
	std::unique_lock lock(assetInfoMutex);
	AssetInfo& info = UidToAssetInfo[AssetUid];

	info.LoadingTask = nullptr;
}

void AssetRegistry::SaveManifest() const
{
	std::unique_lock lock(assetInfoMutex);
	
	std::vector<std::byte> writeBuffer;
	Archive::ArchiveWriter archiveWriter(writeBuffer);
	Archive::Context ctx(nullptr);

	if (!Archive::Serialize(ctx, archiveWriter, UidToAssetInfo))
	{
		LE_ASSERT_DESC(false, ctx.Error.Desc)
		return;
	}

	Path manifestPath = GetContentRoot() / MANIFEST_MAP_PATH;
	if (!SaveFile(manifestPath, writeBuffer))
	{
		LE_ASSERT_DESC(false, "Failed saving manifest file at {}", manifestPath.generic_string())
	}
}

void AssetRegistry::LoadManifest()
{
	Path manifestPath = GetContentRoot()/ MANIFEST_MAP_PATH;
	std::vector<std::byte> readBuffer;
	if (!LoadFile(manifestPath, readBuffer))
	{
		LE_WARN("Failed loading manifest file at {}", manifestPath.generic_string());
		return;
	}

	Archive::ArchiveReader reader(readBuffer);
	Archive::Context ctx(nullptr);

	if (!Archive::Deserialize(ctx, reader, UidToAssetInfo))
	{
		LE_ASSERT_DESC(false, ctx.Error.Desc)
	}

	for (const auto& it : UidToAssetInfo)
	{
		PathToUid[it.second.PathToAsset] = it.first;
	}
}
}
