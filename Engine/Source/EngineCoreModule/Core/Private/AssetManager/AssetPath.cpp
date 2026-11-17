#include "AssetManager/AssetPath.h"

#include "Core.h"
#include "Utilities/StringUtils.h"

namespace LE
{
static constexpr std::string_view PathSeparator(":/");

std::optional<AssetPath> AssetPath::ParseAssetPath(const std::string& OsPath)
{
	if (OsPath.empty())
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath OsPath is empty");
		return std::nullopt;
	}

	const std::size_t mountEnd = OsPath.find_first_of(PathSeparator);
	if (mountEnd == std::string::npos || mountEnd == 0)
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: Invalid mount name");
		return std::nullopt;
	}

	const std::string mountName = OsPath.substr(0, mountEnd);
	if (HasAnyNonAsciiCharacters(mountName))
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: Invalid mount name. Should only contain ASCII characters");
		return std::nullopt;
	}

	if (OsPath.size() == mountEnd + PathSeparator.length())
	{
		LE_ASSERT_DESC(false, "AssetPath::ParseAssetPath: No path present, can be pointing to mount root");
		return std::nullopt;
	}

	const std::string path = OsPath.substr(mountEnd + PathSeparator.length());
	return AssetPath(mountName, path);
}

std::string AssetPath::ToString() const
{
	return MountName + PathSeparator + Path;
}
}
