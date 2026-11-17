#include "Utilities/StringUtils.h"

#include <algorithm>

namespace LE
{

std::wstring CharToWstring(const char* Str)
{
	if (!Str)
	{
		return L"";
	}
	
	size_t sizeNeeded = mbstowcs(nullptr, Str, 0);
	if (sizeNeeded == static_cast<size_t>(-1)) {
		return L"";
	}
    
	// Allocate the wstring buffer and convert
	std::wstring wstr(sizeNeeded, L'\0');
	mbstowcs(&wstr[0], Str, sizeNeeded);
    
	return wstr;
}

static bool HasAnyNonAsciiCharacters(std::string_view Str)
{
	return std::ranges::any_of(Str.begin(), Str.end(), [](const unsigned char c)
							   {
								   return !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
							   });
}
}
