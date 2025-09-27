#pragma once
#include "CoreDefinitions.h"
#include "Math/Math.h"

namespace LE
{
using SharedResourceType = uint32;

template <class Resource>
struct SharedResourceRegistration;

template <class Resource>
struct SharedResourceTypeIdGetter
{
	static constexpr std::string_view TypeName = SharedResourceRegistration<Resource>::Name;
	static constexpr SharedResourceType Value = FNV1AHash(TypeName);
};

#define REGISTER_SHARED_RESOURCE(ResourceClass, ResourceName) \
	template<> \
	struct SharedResourceRegistration<ResourceClass> \
	{ \
		static constexpr std::string_view Name = ResourceName; \
	};
}
