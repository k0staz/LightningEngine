#pragma once
#include <unordered_map>
#include <unordered_set>

#include "AssetCore.h"
#include "Misc/Paths.h"
#include "Misc/Uid.h"
#include "Service/ServiceBase.h"
#include "Service/ServiceRegistry.h"

namespace LE
{
struct AssetInfo
{
	bool IsValid() const
	{
		return AssetUid.IsValid();
	}

	Uid AssetUid;
	AssetId RuntimeId; // Null if unloaded
	std::unordered_set<Uid> Dependencies;
	Path PathToAsset;

	// TODO: Packaging for cooked builds
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetInfo& Value)
{
	if (!Serialize(Ctx, Writer, Value.AssetUid))
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
	void ShutDown() override;

	// Relative path to the content folder
	Uid GetUidFromPath(const Path& AssetPath) const;

	bool IsValidAsset(const Uid& AssetUid) const;

	AssetInfo GetAssetInfo(const Uid& AssetUid) const;
	AssetInfo& GetAssetInfo(const Uid& AssetUid);

	void UpdateAssetRuntimeId(const Uid& AssetUid, AssetId RuntimeId);
	void UpdateAssetPath(const Uid& AssetUid, const Path& AssetPath);

	void SaveManifest() const;

private:
	void LoadManifest();
private:
	std::unordered_map<Path, Uid> PathToUid;
	std::unordered_map<Uid, AssetInfo> UidToAssetInfo;
	// TODO: Handles to files
};

REGISTER_SERVICE_TYPE(AssetRegistry, "AssetRegistry")

}
