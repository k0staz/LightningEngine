#pragma once
#include "CoreDefinitions.h"
#include "Math/Math.h"


namespace LE
{
using EcsComponentType = uint32;

template <typename Component, typename Entity>
struct EcsComponentTraits
{
	using ComponentType = Component;
	using EntityType = Entity;

	static constexpr uint64 PageSize = ENTITY_SPARSE_PAGE;
};

template <class ComponentType>
struct ComponentRegistration;

template <class ComponentType>
struct ComponentTypeIdGetter
{
	static constexpr std::string_view TypeName = ComponentRegistration<ComponentType>::Value;
	static constexpr EcsComponentType Value = FNV1AHash(TypeName);
};

#define ECS_REGISTER_COMPONENT(ComponentType, ComponentName) \
	template<> \
	struct ComponentRegistration<ComponentType> \
	{ \
		static constexpr std::string_view Value = ComponentName; \
	};
}
