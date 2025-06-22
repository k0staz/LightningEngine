#pragma once
#include "Array.h"
#include "CoreDefinitions.h"

namespace LE::StringUtils
{
uint32 ParseString(const String& InString, const String& Delimiter, Array<String>& OutParsedStrings);
}
