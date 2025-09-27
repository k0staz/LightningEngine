#pragma once
#include <atomic>
#include <unordered_set>

#include "Thread.h"
#include "Multithreading/JobNode.h"

#include "UpdatePasses.h"
#include "ECS/EcsComponent.h"
#include "Templates/NonCopyable.h"
#include "Templates/RefCounters.h"

namespace LE
{
#define RENDER_THREAD_FRAME_BEHIND_MAX 2

struct UpdatePass;

class JobScheduler : public NonCopyable
{
public:
	static JobScheduler* Get();

	void Init(int8 WorkerThreadsNum);
	void StartRenderThread();
	void Shutdown();

	void ConstructUpdateGraph();

	void StartFrame();
	void StartFrameRender(Delegate<void(const float)> Delegate);
	void IncrementRenderThreadCount();

	void OnJobBecameAvailable(RefCountingPtr<JobNode> JobNode);
	void OnJobFinished();

	bool AreAllFinished() const;
	void WaitForAll();
	void HelpWorkerThreads(); // Should be called from MT. Do jobs till all are completed

	bool TryStealJobFromThread(uint8 RequestingThreadIdx, RefCountingPtr<JobNode>& OutJob, ThreadType StealingType = ThreadType::Worker);

private:
	void PushJob(RefCountingPtr<JobNode> JobNode);

private:
	JobScheduler()
		: ThreadCount(0)
		  , FrameCounter(0)
	{
	}

	struct GraphBuildContext
	{
		std::unordered_map<SharedResourceType, std::unordered_set<RefCountingPtr<JobNode>>> LastReadingJobsPerResource;
		std::unordered_map<EcsComponentType, std::unordered_set<RefCountingPtr<JobNode>>> LastReadingJobsPerComponent;

		std::unordered_map<SharedResourceType, RefCountingPtr<JobNode>> LastModifyingJobPerResource;
		std::unordered_map<EcsComponentType, RefCountingPtr<JobNode>> LastModifyingJobPerComponent;

		std::unordered_map<RefCountingPtr<JobNode>, std::unordered_set<RefCountingPtr<JobNode>>> AccountedDependencies;

		std::unordered_map<UpdatePassType, std::unordered_set<UpdatePassType>> UpdatePassDependencies;
		std::unordered_map<UpdatePassType, bool> ProcessedUpdatePasses;
	};

	void ConstructUpdateGraphForPass(const UpdatePass* Pass, GraphBuildContext& Context);
	void ConstructUpdateGraphForJobs(const UpdatePass* Pass, GraphBuildContext& Context);
	bool ValidateGraph();

	std::vector<RefCountingPtr<JobNode>> AvailableJobs;
	std::vector<RefCountingPtr<JobNode>> Jobs;

	std::atomic<uint32_t> ActiveJobs;
	std::condition_variable FrameFinishedCV;
	std::mutex FrameFinishedMutex;

	std::atomic<uint32> CurrentThreadForPush;

	uint8 ThreadCount;
	std::vector<Thread> ThreadPool;
	RefCountingPtr<Thread> RenderThread;

	uint64 FrameCounter;
};
}
