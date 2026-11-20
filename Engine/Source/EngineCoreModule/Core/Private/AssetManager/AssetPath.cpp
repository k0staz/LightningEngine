#include "AssetManager/AssetPath.h"

#include "Core.h"
#include "Utilities/StringUtils.h"

namespace LE
{
static constexpr std::string_view MountSeparator(":/");
static constexpr std::string_view PathSeparator("/");
static constexpr std::string_view InvalidNameCharacters(".<>:/|\\?*");

std::optional<AssetPath> AssetPath::ParseAssetPath(const std::string& EnginePath)
{
	if (EnginePath.empty())
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath OsPath is empty");
		return std::nullopt;
	}

	const std::size_t start = 0;
	const std::size_t mountEnd = EnginePath.find_first_of(MountSeparator, start);
	if (mountEnd == std::string::npos || mountEnd == start)
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: Invalid mount name");
		return std::nullopt;
	}

	const std::string mountName = EnginePath.substr(start, mountEnd);
	if (!ValidateMountName(mountName))
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: Invalid mount name");
		return std::nullopt;
	}

	if (EnginePath.size() == mountEnd + MountSeparator.length())
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: No path present, can be pointing to mount root");
		return std::nullopt;
	}

	const std::string path = EnginePath.substr(mountEnd + MountSeparator.length());
	// Normalize path
	std::string normalizePath;
	if (!ValidateAndNormalizeAssetPath(path, normalizePath))
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: Invalid character in the path: {}", path.c_str());
		return std::nullopt;
	}

	return AssetPath(mountName, normalizePath);
}
std::optional<AssetPath> AssetPath::CreateAssetPath(const std::string& InMountName, const std::string& InAssetPath)
{
	std::string normalizedPath;
	if (!ValidateMountName(InMountName) || !ValidateAndNormalizeAssetPath(InAssetPath, normalizedPath))
	{
		LE_ASSERT_DESC(false, "AssetPath::CreateAssetPath: Invalid mount name: {} or Path: {}", InMountName.c_str(), normalizedPath.c_str());
		return std::nullopt;
	}

	return AssetPath(InMountName, normalizedPath);
}

std::string AssetPath::ToString() const
{
	std::string result;
	result.reserve(MountName.size() + MountSeparator.size() + Path.size());
	result.append(MountName);
	result.append(MountSeparator);
	result.append(Path);
	return result;
}
AssetPath::AssetPath(std::string InMountName, std::string InAssetPath)
	: MountName(std::move(InMountName))
	, Path(std::move(InAssetPath))
{
}
bool AssetPath::ValidateMountName(const std::string& InMountName)
{
	return !InMountName.empty() && !HasAnyNonAsciiCharacters(InMountName);
}
bool AssetPath::ValidateAndNormalizeAssetPath(const std::string& InAssetPath, std::string& NormalizedPath)
{
	NormalizedPath.reserve(InAssetPath.size());

	size_t pathStart = InAssetPath.find_first_not_of(PathSeparator);
	while (pathStart != std::string::npos)
	{
		const size_t end = InAssetPath.find_first_of(PathSeparator, pathStart);
		const std::string current = InAssetPath.substr(pathStart, end - pathStart);
		const size_t invalidCharacter = current.find_first_of(InvalidNameCharacters);
		if (invalidCharacter != std::string::npos)
		{
			if (end != std::string::npos || current.at(invalidCharacter) != '.')
			{
				return false;
			}
		}

		NormalizedPath.append(current);
		if (end != std::string::npos)
		{
			NormalizedPath.append(PathSeparator);
		}

		pathStart = InAssetPath.find_first_not_of(PathSeparator, end);
	}

	return true;
}
}
