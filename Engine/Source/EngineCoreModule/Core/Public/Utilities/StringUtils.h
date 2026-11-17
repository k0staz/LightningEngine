#pragma once
#include <string>

namespace LE
{
static std::wstring CharToWstring(const char* Str);
static bool HasAnyNonAsciiCharacters(std::string_view Str);
}
