#pragma once

#include "Math/Color.h"
#include "Multithreading/UpdateJobs.h"
#include "Templates/NonCopyable.h"

namespace LE
{
using UpdatePassType = uint32;

template <class UpdatePass>
struct UpdatePassRegistration;

template <class UpdatePass>
struct UpdatePassTypeIdGetter
{
	static constexpr std::string_view TypeName = UpdatePassRegistration<UpdatePass>::Value;
	static constexpr UpdatePassType Value = FNV1AHash(TypeName);
};

struct UpdatePass
{
	UpdatePass()
	{
		GetUpdatePasses().push_back(this);
	}

	static std::vector<const UpdatePass*>& GetUpdatePasses();

	template<typename UpdatePassT, typename UpdateJobType>
	static void AddJob(const UpdateJobType* Job)
	{
		GetUpdatePassInternal<UpdatePassT>()->AddJob(UpdateJobTypeIdGetter<UpdateJobType>::Value, Job);
	}

	void AddJob(UpdateJobType JobType, const UpdateJob* Job)
	{
		Jobs[JobType] = Job;
	}

	const std::unordered_map<UpdateJobType, const UpdateJob*>& GetUpdateJobs() const
	{
		return Jobs;
	}

	template<typename UpdatePassT>
	static const UpdatePass* GetUpdatePass()
	{
		return GetUpdatePassInternal<UpdatePassT>();
	}

	std::string_view GetName() const
	{
		return Name;
	}

	UpdatePassType GetType() const
	{
		return Type;
	}

	Color GetDebugColor() const
	{
		return DebugColor;
	}

	const std::unordered_set<UpdatePassType>& GetDependsOnPasses() const
	{
		return DependsOn;
	}

	static const UpdatePass* GetUpdatePass(UpdatePassType UpdatePass);

protected:
	template<typename... UpdatePassType>
	void DependsOnPasses()
	{
		((DependsOn.emplace(UpdatePassTypeIdGetter<UpdatePassType>::Value)), ...);
	}

	template<typename UpdatePassType>
	static UpdatePass* GetUpdatePassInternal()
	{
		static UpdatePassType updatePass;
		return &updatePass;
	}

	template<typename UpdatePassType>
	void RegisterToMap()
	{
		GetUpdatePassMap()[UpdatePassTypeIdGetter<UpdatePassType>::Value] = this;
	}

	static std::unordered_map<UpdatePassType, const UpdatePass*>& GetUpdatePassMap();

	std::unordered_set<UpdatePassType> DependsOn;
	std::unordered_map<UpdateJobType, const UpdateJob*> Jobs;
	std::string Name;
	Color DebugColor;
	UpdatePassType Type;
};

#define REGISTER_UPDATE_PASS(UpdatePassType, DebugColorIn, ...) \
	struct UpdatePassType : UpdatePass \
	{ \
		UpdatePassType() \
			: UpdatePass() \
			{ \
			DependsOnPasses<__VA_ARGS__>(); \
			RegisterToMap<UpdatePassType>(); \
			Name = #UpdatePassType; \
			Type = FNV1AHash(Name); \
			DebugColor = DebugColorIn; \
			} \
	}; \
	template<> \
	struct UpdatePassRegistration<UpdatePassType> \
	{ \
		static constexpr std::string_view Value = #UpdatePassType; \
	};
}
