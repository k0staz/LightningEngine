#pragma once

#include "CoreMinimum.h"
#include "SharedResource.h"
#include "ECS/EcsComponent.h"
#include "Misc/Delegate.h"
#include "unordered_set"
#include "ECS/Ecs.h"

namespace LE
{
using UpdateJobType = uint32;

template <class UpdateJob>
struct UpdateJobTypeIdGetter
{
	static constexpr std::string_view TypeName = UpdateJob::Name;
	static constexpr UpdateJobType Value = FNV1AHash(TypeName);
};

struct UpdateJob
{
private:
	friend class JobScheduler;

	enum class Operation : uint8
	{
		Read = 0,
		Write,
		Add,
		Delete,

		Count
	};

	template <typename EcsComponent>
	void AddComponentOperation(Operation Type)
	{
		switch (Type)
		{
		case Operation::Delete:
		{
			ComponentOperations[static_cast<size_t>(Operation::Add)].erase(ComponentTypeIdGetter<EcsComponent>::Value);
		}
		case Operation::Add:
		{
			ComponentOperations[static_cast<size_t>(Operation::Write)].erase(ComponentTypeIdGetter<EcsComponent>::Value);
		}
		case Operation::Write:
		{
			ComponentOperations[static_cast<size_t>(Operation::Read)].erase(ComponentTypeIdGetter<EcsComponent>::Value);
		}
		case Operation::Read:
			break;
		}

		switch (Type)
		{
		case Operation::Read:
			{
			if (ComponentOperations[static_cast<size_t>(Operation::Write)].contains(ComponentTypeIdGetter<EcsComponent>::Value))
			{
				return;
			}
			}
		case Operation::Write:
			{
			if (ComponentOperations[static_cast<size_t>(Operation::Add)].contains(ComponentTypeIdGetter<EcsComponent>::Value))
			{
				return;
			}
			}
		case Operation::Add:
			{
			if (ComponentOperations[static_cast<size_t>(Operation::Delete)].contains(ComponentTypeIdGetter<EcsComponent>::Value))
			{
				return;
			}
			}
		case Operation::Delete:
			break;
		}

		ComponentOperations[static_cast<size_t>(Type)].emplace(ComponentTypeIdGetter<EcsComponent>::Value);
	}

	template <typename Resource>
	void AddResourceOperation(Operation Type)
	{
		switch (Type)
		{
		case Operation::Delete:
		{
			ResourceOperations[static_cast<size_t>(Operation::Add)].erase(SharedResourceTypeIdGetter<Resource>::Value);
		}
		case Operation::Add:
		{
			ResourceOperations[static_cast<size_t>(Operation::Write)].erase(SharedResourceTypeIdGetter<Resource>::Value);
		}
		case Operation::Write:
		{
			ResourceOperations[static_cast<size_t>(Operation::Read)].erase(SharedResourceTypeIdGetter<Resource>::Value);
		}
		case Operation::Read:
			break;
		}

		switch (Type)
		{
		case Operation::Read:
		{
			if (ResourceOperations[static_cast<size_t>(Operation::Write)].contains(SharedResourceTypeIdGetter<Resource>::Value))
			{
				return;
			}
		}
		case Operation::Write:
		{
			if (ResourceOperations[static_cast<size_t>(Operation::Add)].contains(SharedResourceTypeIdGetter<Resource>::Value))
			{
				return;
			}
		}
		case Operation::Add:
		{
			if (ResourceOperations[static_cast<size_t>(Operation::Delete)].contains(SharedResourceTypeIdGetter<Resource>::Value))
			{
				return;
			}
		}
		case Operation::Delete:
			break;
		}

		ResourceOperations[static_cast<size_t>(Type)].emplace(SharedResourceTypeIdGetter<Resource>::Value);
	}

public:
	template <typename... EcsComponent>
	void ReadsComponents()
	{
		((AddComponentOperation<EcsComponent>(Operation::Read)), ...);
		CacheComponentNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void WritesComponents()
	{
		((AddComponentOperation<EcsComponent>(Operation::Write)), ...);
		CacheComponentNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void AddsComponents()
	{
		((AddComponentOperation<EcsComponent>(Operation::Add)), ...);
		CacheComponentNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void DeletesComponents()
	{
		((AddComponentOperation<EcsComponent>(Operation::Delete)), ...);
		CacheComponentNames<EcsComponent...>();
	}

	template <typename... Resource>
	void ReadsResources()
	{
		((AddResourceOperation<Resource>(Operation::Read)), ...);
		CacheResourceNames<Resource...>();
	}

	template <typename... Resource>
	void WritesResources()
	{
		((AddResourceOperation<Resource>(Operation::Write)), ...);
		CacheResourceNames<Resource...>();
	}

	template <typename... Resource>
	void AddsResources()
	{
		((AddResourceOperation<Resource>(Operation::Add)), ...);
		CacheResourceNames<Resource...>();
	}

	template <typename... Resource>
	void DeletesResources()
	{
		((AddResourceOperation<Resource>(Operation::Delete)), ...);
		CacheResourceNames<Resource...>();
	}

	const std::unordered_set<EcsComponentType>& GetReadComponents() const
	{
		return ComponentOperations[static_cast<size_t>(Operation::Read)];
	}

	const std::unordered_set<EcsComponentType>& GetWriteComponents() const
	{
		return ComponentOperations[static_cast<size_t>(Operation::Write)];
	}

	const std::unordered_set<EcsComponentType>& GetAddComponents() const
	{
		return ComponentOperations[static_cast<size_t>(Operation::Add)];
	}

	const std::unordered_set<EcsComponentType>& GetDeleteComponents() const
	{
		return ComponentOperations[static_cast<size_t>(Operation::Delete)];
	}

	const std::unordered_set<SharedResourceType>& GetReadResources() const
	{
		return ResourceOperations[static_cast<size_t>(Operation::Read)];
	}

	const std::unordered_set<SharedResourceType>& GetWriteResources() const
	{
		return ResourceOperations[static_cast<size_t>(Operation::Write)];
	}

	const std::unordered_set<SharedResourceType>& GetAddResources() const
	{
		return ResourceOperations[static_cast<size_t>(Operation::Add)];
	}

	const std::unordered_set<SharedResourceType>& GetDeleteResources() const
	{
		return ResourceOperations[static_cast<size_t>(Operation::Delete)];
	}

	bool IsDeletingJob() const
	{
		return !GetDeleteComponents().empty() || !GetDeleteResources().empty();
	}

	bool IsAddingJob() const
	{
		return !GetAddComponents().empty() || !GetAddResources().empty();
	}

	bool IsWritingJob() const
	{
		return !GetWriteComponents().empty() || !GetWriteResources().empty();
	}

	bool IsReadingJob() const
	{
		return !GetReadComponents().empty() || !GetReadResources().empty();
	}

	virtual std::string_view GetName() const = 0;
	virtual UpdateJobType GetType() const = 0;


	std::string_view GetComponentName(EcsComponentType ComponentType) const
	{
#ifdef DEBUG
		if (ComponentNames.contains(ComponentType))
		{
			return ComponentNames.at(ComponentType);
		}
#endif

		return "";
	}

	std::string_view GetResourceName(SharedResourceType ResourceType) const
	{
#ifdef DEBUG
		if (ResourceNames.contains(ResourceType))
		{
			return ResourceNames.at(ResourceType);
		}
#endif

		return "";
	}

protected:
	std::array<std::unordered_set<EcsComponentType>, static_cast<std::size_t>(Operation::Count)> ComponentOperations;

	std::array<std::unordered_set<SharedResourceType>, static_cast<std::size_t>(Operation::Count)> ResourceOperations;

	Delegate<void(const float)> UpdateFunction;

	template <typename... EcsComponent>
	void CacheComponentNames()
	{
#ifdef DEBUG
		((ComponentNames[ComponentTypeIdGetter<EcsComponent>::Value] = ComponentTypeIdGetter<EcsComponent>::TypeName), ...);
#endif
	}

	template <typename... Resource>
	void CacheResourceNames()
	{
#ifdef DEBUG
		((ResourceNames[SharedResourceTypeIdGetter<Resource>::Value] = SharedResourceTypeIdGetter<Resource>::TypeName), ...);
#endif
	}

#ifdef DEBUG
	// Debug
	std::unordered_map<EcsComponentType, std::string_view> ComponentNames;
	std::unordered_map<SharedResourceType, std::string_view> ResourceNames;
#endif
};

#define EXPAND(...) __VA_ARGS__
#define UNWRAP(x)  EXPAND x

#define REGISTER_OBSERVER_JOB(ObserverJobName, ObserveType, ObservedComponents, FilteredComponents) \
	  struct ObserverJobName##Type : public UpdateJob \
	  { \
		static constexpr std::string_view Name = #ObserverJobName; \
		virtual std::string_view GetName() const override \
		{ \
		return Name; \
		} \
		static constexpr UpdateJobType Type = FNV1AHash(Name); \
		virtual UpdateJobType GetType() const override \
		{ \
		return Type; \
		} \
		ObserverJobName##Type() \
		{ \
		Observer = ObserveComponents<UNWRAP(ObservedComponents)>(ObserveType, ExcludedComponentTypes<UNWRAP(FilteredComponents)>()); \
		UpdateFunction.Attach<&ObserverJobName##Type::TryRunObserver>(this); \
		ReadsComponents<UNWRAP(ObservedComponents)>();\
		} \
		using ObserverType =  EcsObserver<ObservedComponentTypes<UNWRAP(ObservedComponents)>, FilteredComponentTypes<UNWRAP(FilteredComponents)>>; \
		void TryRunObserver(const float) \
		{ \
		if (Observer.IsEmpty()) \
		{ \
			return; \
		} \
		ObserverDelegate(Observer); \
		Observer.ResetObservedEntities(); \
		} \
		Delegate<void(const ObserverType&)>& GetDelegate() \
		{ \
		return ObserverDelegate; \
		} \
		private: \
		Delegate<void(const ObserverType&)>	ObserverDelegate; \
		ObserverType Observer; \
	}; \
	ObserverJobName##Type ObserverJobName;

#define REGISTER_UPDATE_JOB(UpdateJobName) \
	struct UpdateJobName##Type : public UpdateJob \
	{ \
		static constexpr std::string_view Name = #UpdateJobName; \
		virtual std::string_view GetName() const override \
		{ \
		return Name; \
		} \
		static constexpr UpdateJobType Type = FNV1AHash(Name); \
		virtual UpdateJobType GetType() const override \
		{ \
		return Type; \
		} \
		Delegate<void(const float)>& GetDelegate() \
		{ \
		return UpdateFunction; \
		} \
	}; \
	UpdateJobName##Type UpdateJobName;
}
