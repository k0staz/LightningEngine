#pragma once
#include <atomic>
#include <unordered_set>

#include "UpdatePasses.h"
#include "ECS/EcsComponent.h"
#include "Templates/NonCopyable.h"
#include "Templates/RefCounters.h"

namespace LE
{
struct UpdatePass;
}

namespace LE
{
class JobScheduler;

class JobNode : public RefCountableBase
{
	friend JobScheduler;

public:
	JobNode(JobScheduler* InOwner, std::string_view InJobName, UpdateJobType InType, UpdatePassType InPassType)
		: JobName(InJobName)
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

	bool IsReady() const
	{
		return GetCurrentRemainingJobCount() == 0;
	}

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
	std::string_view JobName;
	UpdateJobType Type;
	UpdatePassType PassType;
	JobScheduler* Owner;
	std::atomic_uint JobsTillReady;
	uint32 DefaultDependencies;
};

class JobScheduler : public NonCopyable
{
public:
	static JobScheduler* Get();

	void ConstructUpdateGraph();

	void OnJobBecameAvailable(RefCountingPtr<JobNode> JobNode);
	void OnJobReadyForNextFrame(RefCountingPtr<JobNode> JobNode);

private:
	JobScheduler()
		: IsExecuting(false)
	{
	}

	struct GraphBuildContext
	{
		std::unordered_map<EcsComponentType, RefCountingPtr<JobNode>> LastJobPerComponent;
		std::unordered_map<UpdatePassType, std::unordered_set<UpdatePassType>> UpdatePassDependencies;
		std::unordered_map<UpdatePassType, bool> ProcessedUpdatePasses;
	};

	void ConstructUpdateGraphForPass(const UpdatePass* Pass, GraphBuildContext& Context);
	void ConstructUpdateGraphForJobs(const UpdatePass* Pass, GraphBuildContext& Context);
	bool ValidateGraph();

	std::vector<RefCountingPtr<JobNode>> AvailableJobs;
	std::vector<RefCountingPtr<JobNode>> NextFrameJobs;
	std::vector<RefCountingPtr<JobNode>> Jobs;
	bool IsExecuting;
};
}
