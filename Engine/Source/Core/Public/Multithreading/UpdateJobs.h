#pragma once

#include "CoreMinimum.h"
#include "ECS/EcsComponent.h"
#include "Misc/Delegate.h"
#include "unordered_set"

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
	template <typename... EcsComponent>
	void ReadsComponents()
	{
		((ReadComponents.emplace(ComponentTypeIdGetter<EcsComponent>::Value)), ...);
		CacheNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void WritesComponents()
	{
		((WriteComponents.emplace(ComponentTypeIdGetter<EcsComponent>::Value)), ...);
		CacheNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void AddsComponents()
	{
		((AddComponents.emplace(ComponentTypeIdGetter<EcsComponent>::Value)), ...);
		CacheNames<EcsComponent...>();
	}

	template <typename... EcsComponent>
	void DeletesComponents()
	{
		((DeleteComponents.emplace(ComponentTypeIdGetter<EcsComponent>::Value)), ...);
		CacheNames<EcsComponent...>();
	}

	const std::unordered_set<EcsComponentType>& GetReadComponents() const
	{
		return ReadComponents;
	}

	const std::unordered_set<EcsComponentType>& GetWriteComponents() const
	{
		return WriteComponents;
	}

	const std::unordered_set<EcsComponentType>& GetAddComponents() const
	{
		return AddComponents;
	}

	const std::unordered_set<EcsComponentType>& GetDeleteComponents() const
	{
		return DeleteComponents;
	}

	bool IsDeletingJob() const
	{
		return !DeleteComponents.empty();
	}

	bool IsAddingJob() const
	{
		return !AddComponents.empty();
	}

	bool IsWritingJob() const
	{
		return !WriteComponents.empty();
	}

	bool IsReadingJob() const
	{
		return !ReadComponents.empty();
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

	Delegate<void(const float)> UpdateFunction;

private:
	std::unordered_set<EcsComponentType> ReadComponents;
	std::unordered_set<EcsComponentType> WriteComponents;
	std::unordered_set<EcsComponentType> AddComponents;
	std::unordered_set<EcsComponentType> DeleteComponents;

	template <typename... EcsComponent>
	void CacheNames()
	{
#ifdef DEBUG
		((ComponentNames[ComponentTypeIdGetter<EcsComponent>::Value] = ComponentTypeIdGetter<EcsComponent>::TypeName), ...);
#endif
	}

#ifdef DEBUG
	// Debug
	std::unordered_map<EcsComponentType, std::string_view> ComponentNames;
#endif
};


#define REGISTER_UPDATE_JOB(UpdateJobName) \
	struct UpdateJobName##Type : UpdateJob \
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
	}; \
	UpdateJobName##Type UpdateJobName;
}
