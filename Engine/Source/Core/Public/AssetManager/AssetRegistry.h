#pragma once
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "AssetCore.h"
#include "Misc/Paths.h"
#include "Misc/Uid.h"
#include "Multithreading/AsyncTaskNode.h"
#include "Service/ServiceBase.h"
#include "Service/ServiceRegistry.h"

namespace LE
{
struct AssetInfo
{
	bool IsValid() const
	{
		return AssetUid.IsValid() && TypeId != AssetTypeIdNull;
	}

	Uid AssetUid = {};
	AssetTypeId TypeId = AssetTypeIdNull;
	AssetId RuntimeId = AssetIdNull; // Null if unloaded
	std::unordered_set<Uid> Dependencies;
	RefCountingPtr<AsyncTaskNodeBase> LoadingTask;
	Path PathToAsset;

	// TODO: Packaging for cooked builds
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetInfo& Value)
{
	if (!Serialize(Ctx, Writer, Value.AssetUid))
	{
		return false;
	}
	
	if (!Serialize(Ctx, Writer, Value.TypeId))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.Dependencies))
	{
		return false;
	}

	if (!Serialize(Ctx, Writer, Value.PathToAsset))
	{
		return false;
	}

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, AssetInfo& Value)
{
	if (!Deserialize(Ctx, Reader, Value.AssetUid))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.TypeId))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.Dependencies))
	{
		return false;
	}

	if (!Deserialize(Ctx, Reader, Value.PathToAsset))
	{
		return false;
	}

	return true;
}


class AssetRegistry : public ServiceBase
{
public:
	AssetRegistry() = default;

	void Initialize() override;
	void Shutdown() override;

	// Relative path to the content folder
	Uid GetUidFromPath(const Path& AssetPath) const;

	bool IsValidAsset(const Uid& AssetUid) const;

	AssetInfo GetAssetInfo(const Uid& AssetUid) const;
	// Potentially could be a problem since multiple could be reading that asset info
	// If such case arises the responsible job should provide that it uses this resource
	AssetInfo& GetAssetInfo(const Uid& AssetUid);

	void UpdateAssetRuntimeId(const Uid& AssetUid, AssetId RuntimeId);
	void UpdateAssetPath(const Uid& AssetUid, const Path& AssetPath);

	void AddLoadingTask(const Uid& AssetUid, RefCountingPtr<AsyncTaskNodeBase> LoadingTask);
	void RemoveLoadingTask(const Uid& AssetUid);

	void SaveManifest() const;

private:
	void LoadManifest();
private:
	std::unordered_map<Path, Uid> PathToUid;
	std::unordered_map<Uid, AssetInfo> UidToAssetInfo;
	mutable std::shared_mutex assetInfoMutex;
	// TODO: Handles to files
};

REGISTER_SERVICE_TYPE(AssetRegistry, "AssetRegistry")

}
