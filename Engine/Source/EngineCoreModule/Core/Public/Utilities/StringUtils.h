#pragma once
#include <string>
#include <algorithm>

namespace LE
{
static bool HasAnyNonAsciiCharacters(std::string_view Str)
{
	return std::ranges::any_of(Str.begin(), Str.end(), [](const unsigned char c)
							   {
								   return !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
							   });
}

static bool AreEqualNoCase(std::string_view Lhs, std::string_view Rhs)
{
	return std::ranges::equal(Lhs, Rhs, [](unsigned char c1, unsigned char c2)
	{
		return std::tolower(c1) == std::tolower(c2);
	});
}
}
