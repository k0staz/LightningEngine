#pragma once
#include "CoreDefinitions.h"

namespace LE
{
enum class ComponentChangeType : uint8
{
	None,
	ComponentAdded,
	ComponentRemoved,
	ComponentUpdated
};
}
