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
struct UpdatePass;
}

namespace LE
{
class JobScheduler : public NonCopyable
{
public:
	static JobScheduler* Get();

	void Init(int WorkerThreadsNum);
	void Shutdown();

	void ConstructUpdateGraph();

	void StartFrame();
	void OnJobBecameAvailable(RefCountingPtr<JobNode> JobNode);
	void OnJobFinished();

	bool AreAllFinished() const;
	void WaitForAll();

	bool TryStealJobFromThread(uint8 RequestingThreadIdx, RefCountingPtr<JobNode>& OutJob);

private:
	void PushJob(RefCountingPtr<JobNode> JobNode);

private:
	JobScheduler()
		:ThreadCount(0)
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
	std::vector<RefCountingPtr<JobNode>> Jobs;

	std::atomic<uint32_t> ActiveJobs;
	std::condition_variable FrameFinishedCV;
	std::mutex FrameFinishedMutex;

	std::atomic<uint32> CurrentThreadForPush;

	uint32 ThreadCount;
	std::vector<Thread> ThreadPool;
};
}
