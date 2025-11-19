#pragma once

#include "Misc/Traits.h"

namespace LE
{
using EcsEntity = uint32;

template<typename Type>
struct EcsTraits : IdTraitsInterpreter<IdTraits<Type>>
{
	static constexpr uint64 PageSize = ENTITY_SPARSE_PAGE;
};

inline constexpr NullId EcsEntityNull{};
}
