#pragma once
#include <filesystem>

namespace LE
{
	using Path = std::filesystem::path;

	Path GetEngineRoot();
}
