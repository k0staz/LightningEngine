#include "Multithreading/JobScheduler.h"

#include "Multithreading/UpdatePasses.h"
#include "Multithreading/Utils/JobVisualizer.h"

namespace LE
{
static JobScheduler* gJobScheduler = nullptr;


JobScheduler* JobScheduler::Get()
{
	if (!gJobScheduler)
	{
		gJobScheduler = new JobScheduler;
	}

	return gJobScheduler;
}

void JobScheduler::Init(int8 WorkerThreadsNum)
{
	if (WorkerThreadsNum <= 0)
	{
		return;
	}
	ThreadCount = WorkerThreadsNum;

	ConstructUpdateGraph();
	LE_INFO("-------------------------Spawning worker threads-------------------------");
	ThreadPool.reserve(WorkerThreadsNum);
	for (uint8 i = 0; i < static_cast<uint8>(WorkerThreadsNum); ++i)
	{
		const std::string threadName = std::format("Worker Thread {}", i);
		ThreadPool.emplace_back(i + 1, threadName, ThreadType::Worker, this);
		LE_INFO("Thread {} was created", threadName);
		ThreadPool[i].Start();
	}
	LE_INFO("-------------------------Finished Spawning worker threads-------------------------");
}

void JobScheduler::StartRenderThread()
{
	LE_INFO("-------------------------Spawning render thread-------------------------");
	const std::string renderThreadName = "Render Thread";
	RenderThread = new Thread(-1, renderThreadName, ThreadType::Render, this);
	RenderThread->Start();
	LE_INFO("-------------------------Finished Spawning render thread-------------------------");
}

void JobScheduler::Shutdown()
{
	for (Thread& thread : ThreadPool)
	{
		thread.Stop();
	}

	RenderThread->Stop();
}

void JobScheduler::ConstructUpdateGraph()
{
	AvailableJobs.clear();
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

void JobScheduler::StartFrame()
{
	++FrameCounter;
	ActiveJobs.store(0);
	CurrentThreadForPush.store(0);
	for (auto& workerThread : ThreadPool)
	{
		workerThread.IncrementFrameCounter();
	}

	for (auto& job : AvailableJobs)
	{
		PushJob(job);
	}
}

void JobScheduler::StartFrameRender(Delegate<void(const float)> Delegate)
{
	// TODO: This needs to be reworked once actual multithreading for render part is done
	RefCountingPtr<JobNode> job = new JobNode(nullptr, "Render Kick-Off job", Delegate, UpdateJobType(), UpdatePassType());
	RenderThread->PushJob(job);
}

void JobScheduler::IncrementRenderThreadCount()
{
	RenderThread->IncrementFrameCounter();
}

void JobScheduler::OnJobBecameAvailable(RefCountingPtr<JobNode> JobNode)
{
	PushJob(JobNode);
}

void JobScheduler::OnJobFinished()
{
	if (ActiveJobs.fetch_sub(1, std::memory_order_acq_rel) == 1)
	{
		std::lock_guard lock(FrameFinishedMutex);
		FrameFinishedCV.notify_all();
	}
}

bool JobScheduler::AreAllFinished() const
{
	return ActiveJobs.load(std::memory_order_acquire) == 0;
}

void JobScheduler::WaitForAll()
{
	std::unique_lock lock(FrameFinishedMutex);
	FrameFinishedCV.wait(lock, [this] { return ActiveJobs.load(std::memory_order_acquire) == 0; });
}

void JobScheduler::HelpWorkerThreads()
{
	uint8 threadIdxToSteal = 0;

	RefCountingPtr<JobNode> currentJob = nullptr;
	while (TryStealJobFromThread(threadIdxToSteal, currentJob))
	{
		currentJob->Execute();
		threadIdxToSteal = (threadIdxToSteal + 1) % ThreadCount;
	}
}

bool JobScheduler::TryStealJobFromThread(uint8 RequestingThreadIdx, RefCountingPtr<JobNode>& OutJob, ThreadType StealingType)
{
	uint8 threadIdxToSteal = (RequestingThreadIdx + 1) % ThreadCount;
	while (threadIdxToSteal != RequestingThreadIdx)
	{
		if (ThreadPool[threadIdxToSteal].TryStealJob(OutJob))
		{
			return true;
		}
		threadIdxToSteal = (threadIdxToSteal + 1) % ThreadCount;
	}

	return false;
}

void JobScheduler::PushJob(RefCountingPtr<JobNode> JobNode)
{
	ActiveJobs.fetch_add(1, std::memory_order_acq_rel);

	uint32 idx = CurrentThreadForPush.fetch_add(1, std::memory_order_acq_rel) + 1;
	for (uint32 i = 0; i != ThreadCount; ++i)
	{
		if (ThreadPool[(i + idx) % ThreadCount].TryPushJob(JobNode))
		{
			return;
		}
	}

	ThreadPool[idx % ThreadCount].PushJob(JobNode);
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

	// This is to remove the number of dependency links, if it was accounted by predecessors
	auto addDependency = [&Context](RefCountingPtr<JobNode>& prevJob, RefCountingPtr<JobNode>& dependentJob)
	{
		if (!Context.AccountedDependencies[dependentJob].contains(prevJob))
		{
			Context.AccountedDependencies[dependentJob].insert(Context.AccountedDependencies[prevJob].begin(),
			                                                   Context.AccountedDependencies[prevJob].end());
			Context.AccountedDependencies[dependentJob].emplace(prevJob);
			prevJob->AddDependentJob(*dependentJob);
		}

		for (auto& accountedJob : Context.AccountedDependencies[prevJob])
		{
			if (Context.AccountedDependencies[dependentJob].contains(accountedJob))
			{
				accountedJob->RemoveDependentJob(*dependentJob);
			}
		}
	};

	auto setupComponentDependencyFunc = [&Context, &addDependency](RefCountingPtr<JobNode>& job,
	                                               const std::unordered_set<EcsComponentType>& components, bool isReading)
	{
		for (EcsComponentType component : components)
		{
			if (!isReading)
			{
				auto itReading = Context.LastReadingJobsPerComponent.find(component);
				if (itReading != Context.LastReadingJobsPerComponent.find(component))
				{
					for (RefCountingPtr<JobNode> readingJob : itReading->second)
					{
						addDependency(readingJob, job);
					}

					return;
				}
			}

			auto it = Context.LastModifyingJobPerComponent.find(component);
			if (it != Context.LastModifyingJobPerComponent.end())
			{
				addDependency(it->second, job);
			}
		}
	};

	auto setupResourceDependencyFunc = [&Context, &addDependency](RefCountingPtr<JobNode>& job,
	                                              const std::unordered_set<SharedResourceType>& resources, bool isReading)
	{
		for (SharedResourceType component : resources)
		{
			if (!isReading)
			{
				auto itReading = Context.LastReadingJobsPerResource.find(component);
				if (itReading != Context.LastReadingJobsPerResource.find(component))
				{
					for (RefCountingPtr<JobNode> readingJob : itReading->second)
					{
						addDependency(readingJob, job);
					}

					return;
				}
			}

			auto it = Context.LastModifyingJobPerResource.find(component);
			if (it != Context.LastModifyingJobPerResource.end())
			{
				addDependency(it->second, job);
			}
		}
	};

	auto updateComponentLastJobsFunc = [&Context](RefCountingPtr<JobNode>& job,
	                                              const std::unordered_set<EcsComponentType>& components, bool isReading)
	{
		for (EcsComponentType component : components)
		{
			if (!isReading)
			{
				Context.LastModifyingJobPerComponent[component] = job;
				Context.LastReadingJobsPerComponent.erase(component);
			}
			else
			{
				Context.LastReadingJobsPerComponent[component].emplace(job);
			}
		}
	};

	auto updateResourceLastJobsFunc = [&Context](RefCountingPtr<JobNode>& job,
	                                             const std::unordered_set<SharedResourceType>& resources, bool isReading)
	{
		for (SharedResourceType component : resources)
		{
			if (!isReading)
			{
				Context.LastModifyingJobPerResource[component] = job;
				Context.LastReadingJobsPerResource.erase(component);
			}
			else
			{
				Context.LastReadingJobsPerResource[component].emplace(job);
			}
		}
	};

	auto scheduleDependencyFunc = [&setupResourceDependencyFunc, &setupComponentDependencyFunc, &updateComponentLastJobsFunc,
			&updateResourceLastJobsFunc, &Pass, this](const std::vector<const UpdateJob*>& jobs)
	{
		for (const UpdateJob* job : jobs)
		{
			RefCountingPtr<JobNode> jobNode = new JobNode(this, job->GetName(), job->UpdateFunction, job->GetType(), Pass->GetType());
			Jobs.push_back(jobNode);

			// Setup dependencies
			setupComponentDependencyFunc(jobNode, job->GetDeleteComponents(), false);
			setupComponentDependencyFunc(jobNode, job->GetAddComponents(), false);
			setupComponentDependencyFunc(jobNode, job->GetWriteComponents(), false);
			setupComponentDependencyFunc(jobNode, job->GetReadComponents(), true);

			setupResourceDependencyFunc(jobNode, job->GetDeleteResources(), false);
			setupResourceDependencyFunc(jobNode, job->GetAddResources(), false);
			setupResourceDependencyFunc(jobNode, job->GetWriteResources(), false);
			setupResourceDependencyFunc(jobNode, job->GetReadResources(), true);

			updateComponentLastJobsFunc(jobNode, job->GetDeleteComponents(), false);
			updateComponentLastJobsFunc(jobNode, job->GetAddComponents(), false);
			updateComponentLastJobsFunc(jobNode, job->GetWriteComponents(), false);
			updateComponentLastJobsFunc(jobNode, job->GetReadComponents(), true);

			updateResourceLastJobsFunc(jobNode, job->GetDeleteResources(), false);
			updateResourceLastJobsFunc(jobNode, job->GetAddResources(), false);
			updateResourceLastJobsFunc(jobNode, job->GetWriteResources(), false);
			updateResourceLastJobsFunc(jobNode, job->GetReadResources(), true);

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
	std::unordered_map<UpdateJobType, uint32> counter;
	std::vector<RefCountingPtr<JobNode>> currentJobs = AvailableJobs;

	uint32 executed = 0;
	while (!currentJobs.empty())
	{
		RefCountingPtr<JobNode> job = currentJobs.back();
		currentJobs.pop_back();
		++executed;

		for (auto& dependent : job->GetDependentJobs())
		{
			UpdateJobType type = dependent->GetType();
			if (!counter.contains(type))
			{
				counter[type] = dependent->GetDefaultRemainingJobCount();
			}
			--counter[type];
			if (counter[type] == 0)
			{
				currentJobs.push_back(dependent);
			}
		}
	}

	return executed == Jobs.size();
}
}
