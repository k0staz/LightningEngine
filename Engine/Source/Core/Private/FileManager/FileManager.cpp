#include "FileManager/FileManager.h"

#include <fstream>

namespace LE
{
String LoadShaderFile(const Path& filePath)
{
	std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string content(static_cast<size_t>(size), '\0');
	file.read(content.data(), size);

	return content;
}
}
