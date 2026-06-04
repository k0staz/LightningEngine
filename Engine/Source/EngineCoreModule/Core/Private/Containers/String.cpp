#include "Containers/String.h"

namespace LE::StringUtils
{
uint32 ParseString(const String& InString, const String& Delimiter, Array<String>& OutParsedStrings)
{
	OutParsedStrings.clear();

	if (InString.empty())
	{
		return 0;
	}

	size_t posStart = 0;
	size_t posEnd = 0;
	while ((posEnd = InString.find(Delimiter, posStart)) != String::npos)
	{
		OutParsedStrings.push_back(InString.substr(posStart, posEnd));
		posStart = posEnd + Delimiter.length();
	}
	OutParsedStrings.push_back(InString.substr(posStart, InString.length()));

	return static_cast<uint32>(OutParsedStrings.size());
}
}
