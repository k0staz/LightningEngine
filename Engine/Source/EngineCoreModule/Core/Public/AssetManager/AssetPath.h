#pragma once
#include "Archive/ArchiveIO.h"
#include "Utilities/StringUtils.h"


#include <optional>
#include <string>

namespace LE
{
struct AssetPath
{
	AssetPath() = default;

	static std::optional<AssetPath> ParseAssetPath(const std::string& EnginePath);
	static std::optional<AssetPath> CreateAssetPath(const std::string& InMountName, const std::string& InAssetPath);
	std::string ToString() const;

	bool operator==(const AssetPath& Other) const
	{
		return AreEqualNoCase(MountName, Other.MountName) && AreEqualNoCase(Path, Other.Path);
	}

	std::string_view GetMountName() const
	{
		return MountName;
	}

	std::string_view GetPath() const
	{
		return Path;
	}

private:
	AssetPath(std::string InMountName, std::string InAssetPath);

	static bool ValidateMountName(const std::string& InMountName);
	static bool ValidateAndNormalizeAssetPath(const std::string& InAssetPath, std::string& NormalizedPath);

	std::string MountName;
	std::string Path;
};

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveWriter& Writer, const AssetPath& Value)
{
	if (!Archive::Serialize(Ctx, Writer, Value.ToString()))
	{
		return false;
	}

	return true;
}

inline bool InvokeArchive(Archive::Context& Ctx, Archive::ArchiveReader& Reader, AssetPath& Value)
{
	std::string assetPathString;
	if (!Archive::Deserialize(Ctx, Reader, assetPathString))
	{
		return false;
	}
	std::optional<AssetPath> result = AssetPath::ParseAssetPath(assetPathString);
	if (!result)
	{
		return false;
	}

	Value = *result;

	return true;
}
}

template<>
struct std::hash<LE::AssetPath>
{
	std::size_t operator()(const LE::AssetPath& AssetPath) const noexcept
	{
		const std::size_t h1 = std::hash<std::string_view>{}(AssetPath.GetMountName());
		const std::size_t h2 = std::hash<std::string_view>{}(AssetPath.GetPath());
		return h1 ^ h2;
	}
};