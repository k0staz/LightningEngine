#pragma once

#include <unordered_set>

#include "AsyncNode.h"
#include "Templates/RefCounters.h"
#include "UpdatePasses.h"

namespace LE
{
class JobScheduler;

// Will probably need to split into Gameplay Work and Render Work
class JobNode : public AsyncNode
{
	friend JobScheduler;

public:
	JobNode(JobScheduler* InOwner, std::string_view InJobName, Delegate<void(const float)> UpdateFunction, UpdateJobType InType,
	        UpdatePassType InPassType)
		: Function(UpdateFunction)
		  , JobName(InJobName)
		  , Type(InType)
		  , PassType(InPassType)
		  , Owner(InOwner)
		  , DefaultDependencies(0)
	{
	}

	void AddDependentJob(JobNode& Job)
	{
		if (DependentJobs.contains(&Job))
		{
			return;
		}

		DependentJobs.emplace(&Job);
		Job.IncrementDependencyCounter();
		++Job.DefaultDependencies;
	}

	void RemoveDependentJob(JobNode& Job)
	{
		if (!DependentJobs.contains(&Job))
		{
			return;
		}

		DependentJobs.erase(&Job);
		Job.DecrementDependencyCounter();
		--Job.DefaultDependencies;
	}

	bool IsReady() const
	{
		return GetCurrentRemainingJobCount() == 0;
	}

	void Execute() override;

	uint32 GetCurrentRemainingJobCount() const
	{
		return JobsTillReady.load(std::memory_order_acquire);
	}

	uint32 GetDefaultRemainingJobCount() const
	{
		return DefaultDependencies;
	}

	const std::unordered_set<RefCountingPtr<JobNode>>& GetDependentJobs() const
	{
		return DependentJobs;
	}

	std::string_view GetName() const
	{
		return JobName;
	}

	UpdateJobType GetType() const
	{
		return Type;
	}

	UpdatePassType GetUpdatePassType() const
	{
		return PassType;
	}

protected:
	void IncrementDependencyCounter();
	void DecrementDependencyCounter();
	void OnCompleted();

private:
	std::unordered_set<RefCountingPtr<JobNode>> DependentJobs;
	Delegate<void(const float)> Function;
	std::string_view JobName;
	UpdateJobType Type;
	UpdatePassType PassType;
	JobScheduler* Owner;
	std::atomic_uint JobsTillReady;
	uint32 DefaultDependencies;
};
}
