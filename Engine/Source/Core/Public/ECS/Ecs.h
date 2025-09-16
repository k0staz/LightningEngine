#pragma once

#include "CoreDefinitions.h"
#include "EcsComponent.h"
#include "EcsEntity.h"
#include "EcsModule.h"
#include "EcsObserver.h"

namespace LE
{
void RegisterECSModule(UniquePtr<ECSModule> Module);
ECSModule& GetECSModule();

template<typename ComponentType>
using ComponentStorageForType = EcsComponentStorage<ComponentType, EcsEntity>;

template<typename ...ComponentType>
using ObservedComponentTypes = IncludedComponentTypes<EcsComponentStorage<ComponentType, EcsEntity>...>;

template<typename ...ComponentType>
using FilteredComponentTypes = ExcludedComponentTypes<EcsComponentStorage<ComponentType, EcsEntity>...>;

static EcsEntity CreateEntity()
{
	return GetECSModule().GetRegistry()->CreateEntity();
}

static bool IsEntityValid(const EcsEntity Entity)
{
	return GetECSModule().GetRegistry()->IsEntityValid(Entity);
}

static bool DeleteEntityByEntityHandle(const EcsEntity Entity)
{
	if (IsEntityValid(Entity))
	{
		GetECSModule().GetRegistry()->DeleteEntity(Entity);
		return true;
	}

	return false;
}

template <typename ComponentType, typename... ComponentArgs>
static ComponentType& AddComponentToEntity(const EcsEntity Entity, ComponentArgs&&... Args)
{
	return GetECSModule().GetRegistry()->AddComponentToEntity<ComponentType>(Entity, std::forward<ComponentArgs>(Args)...);
}

template <typename... ComponentType>
static bool HasAllComponents(const EcsEntity Entity)
{
	return GetECSModule().GetRegistry()->HasAllComponents<ComponentType...>(Entity);
}

template <typename... ComponentType>
static bool HasAnyComponents(const EcsEntity Entity)
{
	return GetECSModule().GetRegistry()->HasAnyComponents<ComponentType...>(Entity);
}

template <typename... ComponentType>
static decltype(auto) GetComponent(const EcsEntity Entity)
{
	return GetECSModule().GetRegistry()->GetComponent<ComponentType...>(Entity);
}

template <typename ComponentType, typename... OtherComponents>
static void DeleteComponent(const EcsEntity Entity)
{
	GetECSModule().GetRegistry()->DeleteComponent<ComponentType, OtherComponents...>(Entity);
}

template <typename... ComponentType, typename... ExcludedComponents>
static EcsStorageView<IncludedComponentTypes<ComponentStorageForType<ComponentType>...>, ExcludedComponentTypes<ComponentStorageForType<ExcludedComponents>...>>
	ViewComponents(ExcludedComponentTypes<ExcludedComponents...>  = ExcludedComponentTypes{})
{
	return GetECSModule().GetRegistry()->View<ComponentType...>(ExcludedComponentTypes<ExcludedComponents...>{});
}

template <typename... ComponentType, typename... ExcludedComponents>
static EcsObserver<IncludedComponentTypes<ComponentStorageForType<ComponentType>...>, ExcludedComponentTypes<ComponentStorageForType<ExcludedComponents>...>>
	ObserveComponents(ComponentChangeType InObserverType, ExcludedComponentTypes<ExcludedComponents...> = ExcludedComponentTypes{})
{
	return GetECSModule().GetRegistry()->Observe<ComponentType...>(InObserverType, ExcludedComponentTypes<ExcludedComponents...>{});
}
}
