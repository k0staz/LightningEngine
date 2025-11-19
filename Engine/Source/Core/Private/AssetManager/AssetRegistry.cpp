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

void AssetRegistry::ShutDown()
{
}

Uid AssetRegistry::GetUidFromPath(const Path& AssetPath) const
{
	if (PathToUid.contains(AssetPath))
	{
		return PathToUid.at(AssetPath);
	}

	return {};
}

bool AssetRegistry::IsValidAsset(const Uid& AssetUid) const
{
	return AssetUid.IsValid() && UidToAssetInfo.contains(AssetUid);
}

AssetInfo AssetRegistry::GetAssetInfo(const Uid& AssetUid) const
{
	if (IsValidAsset(AssetUid))
	{
		return UidToAssetInfo.at(AssetUid);
	}

	return {};
}

AssetInfo& AssetRegistry::GetAssetInfo(const Uid& AssetUid)
{
	return UidToAssetInfo[AssetUid];
}

void AssetRegistry::UpdateAssetRuntimeId(const Uid& AssetUid, AssetId RuntimeId)
{
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

	AssetInfo& info = GetAssetInfo(AssetUid);
	PathToUid.erase(info.PathToAsset);

	info.PathToAsset = AssetPath;
	PathToUid[AssetPath] = AssetUid;
}

void AssetRegistry::SaveManifest() const
{
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
		LE_ASSERT_DESC(false, "Failed loading manifest file at {}", manifestPath.generic_string())
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
