#pragma once
#include <fstream>

#include "Core.h"
#include "Containers/String.h"
#include "Misc/Paths.h"

namespace LE
{
String LoadShaderFile(const Path& filePath);

// TODO: Replace with interface
inline bool LoadFile(const Path& FilePath, std::vector<std::byte>& OutData)
{
	OutData.clear();
	
	std::ifstream stream(FilePath, std::ios::binary | std::ios::ate);
	if (!stream.is_open())
	{
		LE_ASSERT_DESC(false, "Failed to open file for read at {}", FilePath.generic_string())
		return false;
	}

	auto size = stream.tellg();
	OutData.resize(size);
	stream.seekg(0);
	stream.read(reinterpret_cast<char*>(OutData.data()), size);
	return true;
}

inline bool SaveFile(const Path& FilePath, const std::vector<std::byte>& Data)
{
	std::ofstream stream(FilePath, std::ios::binary, std::ios::trunc);
	if (!stream.is_open())
	{
		LE_ASSERT_DESC(false, "Failed to open file for write at {}", FilePath.generic_string())
		return false;
	}

	stream.write(reinterpret_cast<const char*>(Data.data()),static_cast<std::streamsize>(Data.size()));
	stream.flush();
	return true;
}
}
