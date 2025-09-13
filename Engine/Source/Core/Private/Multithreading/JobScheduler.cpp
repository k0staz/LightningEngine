#include "Multithreading/JobScheduler.h"

#include "Multithreading/UpdatePasses.h"
#include "Multithreading/Utils/JobVisualizer.h"

namespace LE
{
static JobScheduler* gJobScheduler = nullptr;

void JobNode::IncrementDependencyCounter()
{
	JobsTillReady.fetch_add(1, std::memory_order_relaxed);
}

void JobNode::DecrementDependencyCounter()
{
	uint32 newValue = JobsTillReady.fetch_sub(1, std::memory_order_acq_rel) - 1;
	if (newValue == 0)
	{
		Owner->OnJobBecameAvailable(this);
	}
}

void JobNode::OnCompleted()
{
	for (auto& dependentJob : DependentJobs)
	{
		dependentJob->DecrementDependencyCounter();
	}
	JobsTillReady.store(DefaultDependencies);
	Owner->OnJobReadyForNextFrame(this);
}

JobScheduler* JobScheduler::Get()
{
	if (!gJobScheduler)
	{
		gJobScheduler = new JobScheduler;
	}

	return gJobScheduler;
}

void JobScheduler::ConstructUpdateGraph()
{
	AvailableJobs.clear();
	NextFrameJobs.clear();
	Jobs.clear();

	std::vector<const UpdatePass*>& updatePasses = UpdatePass::GetUpdatePasses();

	LE_INFO("-------------------------Starting Update graph construction-------------------------");
	GraphBuildContext context;
	for (const auto& pass : updatePasses)
	{
		ConstructUpdateGraphForPass(pass, context);
	}
	LE_ASSERT_DESC(ValidateGraph(), "Constructed graph is invalid")
	LE::JobVisualizer visualizer(Jobs);
	visualizer.Dump(GetEngineRoot().parent_path() / "Debug" / "UpdatePass.dot");
	LE_INFO("-------------------------Finished Update graph construction-------------------------");
}

void JobScheduler::OnJobBecameAvailable(RefCountingPtr<JobNode> JobNode)
{
	// TODO: We need mutex here, or do it per thread
	AvailableJobs.push_back(JobNode);
}

void JobScheduler::OnJobReadyForNextFrame(RefCountingPtr<JobNode> JobNode)
{
	// TODO: We need mutex here, or do it per thread
	NextFrameJobs.emplace_back(JobNode);
}

void JobScheduler::ConstructUpdateGraphForPass(const UpdatePass* Pass, GraphBuildContext& Context)
{
	UpdatePassType passType = Pass->GetType();
	if (Context.ProcessedUpdatePasses.contains(passType) && Context.ProcessedUpdatePasses[passType])
	{
		return;
	}

	std::unordered_set<UpdatePassType>& dependsOn = Context.UpdatePassDependencies[passType];
	Context.ProcessedUpdatePasses[passType] = false;
	for (const UpdatePassType& requiredPass : Pass->GetDependsOnPasses())
	{
		dependsOn.emplace(requiredPass);

		const UpdatePass* requiredPtr = UpdatePass::GetUpdatePass(requiredPass);

#ifdef DEBUG
		if (Context.UpdatePassDependencies[requiredPass].contains(passType))
		{
			LE_ASSERT_DESC(false, "Circular dependency between {} and {}", Pass->GetName(), requiredPtr->GetName())
			break;
		}
#endif

		ConstructUpdateGraphForPass(requiredPtr, Context);
	}

	LE_INFO("	Processing Update Pass: {}", Pass->GetName());
	ConstructUpdateGraphForJobs(Pass, Context);
	Context.ProcessedUpdatePasses[passType] = true;
	LE_INFO("	Finished processing Update Pass: {}", Pass->GetName());
}

void JobScheduler::ConstructUpdateGraphForJobs(const UpdatePass* Pass, GraphBuildContext& Context)
{
	std::vector<const UpdateJob*> deleteJobs;
	std::vector<const UpdateJob*> addJobs;
	std::vector<const UpdateJob*> WriteJobs;
	std::vector<const UpdateJob*> ReadJobs;
	auto populateFunc = [](std::vector<const UpdateJob*>& jobsArray, const UpdateJob* job)
	{
		jobsArray.push_back(job);
	};

	// Populate jobs per components
	for (const auto& jobPair : Pass->GetUpdateJobs())
	{
		const UpdateJob* job = jobPair.second;

		if (job->IsDeletingJob())
		{
			populateFunc(deleteJobs, job);
			continue;
		}

		if (job->IsAddingJob())
		{
			populateFunc(addJobs, job);
			continue;
		}

		if (job->IsWritingJob())
		{
			populateFunc(WriteJobs, job);
			continue;
		}

		if (job->IsReadingJob())
		{
			populateFunc(ReadJobs, job);
			continue;
		}

		LE_WARN("Detected job: {} which doesn't do anything, it will be skipped", job->GetName());
	}

	auto setupDependencyFunc = [&Context](RefCountingPtr<JobNode>& job,
	                                      const std::unordered_set<EcsComponentType>& components)
	{
		for (EcsComponentType component : components)
		{
			auto it = Context.LastJobPerComponent.find(component);
			if (it != Context.LastJobPerComponent.end())
			{
				it->second->AddDependentJob(*job);
			}
		}
	};

	auto updateLastJobsFunc = [&Context](RefCountingPtr<JobNode>& job,
	                                     const std::unordered_set<EcsComponentType>& components)
	{
		for (EcsComponentType component : components)
		{
			Context.LastJobPerComponent[component] = job;
		}
	};

	auto scheduleDependencyFunc = [&setupDependencyFunc, &updateLastJobsFunc, &Pass, this](const std::vector<const UpdateJob*>& jobs)
	{
		for (const UpdateJob* job : jobs)
		{
			RefCountingPtr<JobNode> jobNode = new JobNode(this, job->GetName(), job->GetType(), Pass->GetType());
			Jobs.push_back(jobNode);

			// Setup dependencies
			setupDependencyFunc(jobNode, job->GetDeleteComponents());
			setupDependencyFunc(jobNode, job->GetAddComponents());
			setupDependencyFunc(jobNode, job->GetWriteComponents());
			setupDependencyFunc(jobNode, job->GetReadComponents());

			updateLastJobsFunc(jobNode, job->GetDeleteComponents());
			updateLastJobsFunc(jobNode, job->GetAddComponents());
			updateLastJobsFunc(jobNode, job->GetWriteComponents());
			updateLastJobsFunc(jobNode, job->GetReadComponents());

			if (jobNode->IsReady())
			{
				AvailableJobs.emplace_back(jobNode);
			}

			LE_INFO("		Job {} is scheduled", job->GetName());
		}
	};

	// 1st - Delete
	// 2nd - Add
	// 3rd - Write
	// 4th - Read
	scheduleDependencyFunc(deleteJobs);
	scheduleDependencyFunc(addJobs);
	scheduleDependencyFunc(WriteJobs);
	scheduleDependencyFunc(ReadJobs);
}

bool JobScheduler::ValidateGraph()
{
	LE_ASSERT_DESC(!IsExecuting, "Don't validate graph during execution")

	uint32 jobsExecuted = 0;
	while (!AvailableJobs.empty())
	{
		++jobsExecuted;
		RefCountingPtr<JobNode> job = AvailableJobs.back();
		AvailableJobs.pop_back();
		job->OnCompleted();
	}

	return jobsExecuted == Jobs.size();
}
}
