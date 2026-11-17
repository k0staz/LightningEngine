#pragma once
#include <optional>
#include <string>

namespace LE
{
struct AssetPath
{
	static std::optional<AssetPath> ParseAssetPath(const std::string& OsPath);
	std::string ToString() const;

	std::string MountName;
	std::string Path;
};
}
