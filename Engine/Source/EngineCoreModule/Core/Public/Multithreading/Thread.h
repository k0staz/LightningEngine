#pragma once
#include <thread>
#include <deque>
#include <mutex>
#include <semaphore>

#include "AsyncTaskNode.h"
#include "JobNode.h"
#include "Templates/RefCounters.h"

namespace LE
{
class JobScheduler;
}

namespace LE
{
enum class ThreadType : uint8_t
{
	Worker = 1,
	Render,
	Task
};

class Thread : public RefCountableBase
{
public:
	static bool IsMainThread();
	static bool IsRenderThread();
	static bool IsTaskThread();
	static int8 GetWorkerThreadIndex();
	static int8 GetWorkerTaskThreadIndex();


	Thread(int8 InIndex, std::string InName, ThreadType InType, JobScheduler* InOwner, int8 InTaskIndex = -1)
		: Index(InIndex)
		  , TaskIndex(InTaskIndex)
		  , Type(InType)
		  , Owner(InOwner)
		  , Name(std::move(InName))
	{}

	Thread(const Thread&) = delete;

	Thread(Thread&& Other) noexcept
	{
		Index = Other.Index;
		Type = Other.Type;
		Owner = Other.Owner;
		Name = Other.Name;
		std::swap(ThreadImpl, Other.ThreadImpl);
		std::swap(LocalQueue, Other.LocalQueue);
	}

	Thread& operator=(const Thread&) = delete;

	Thread& operator=(Thread&& Other) noexcept
	{
		std::swap(Index, Other.Index);
		std::swap(Type, Other.Type);
		std::swap(Owner, Other.Owner);
		std::swap(Name, Other.Name);
		std::swap(ThreadImpl, Other.ThreadImpl);
		std::swap(LocalQueue, Other.LocalQueue);
		return *this;
	}

	~Thread() override = default;

	ThreadType GetType() const
	{
		return Type;
	}

	void Start();
	void Stop();

	void Main();

	bool TryPushJob(RefCountingPtr<JobNode> JobToAdd);
	void PushJob(RefCountingPtr<JobNode> JobToAdd);
	bool TryStealJob(RefCountingPtr<AsyncNode>& JobOut);

	bool TryPushTask(RefCountingPtr<AsyncTaskNodeBase> TaskToAdd);
	void PushTask(RefCountingPtr<AsyncTaskNodeBase> TaskToAdd);

	void IncrementFrameCounter();
	uint64 GetCurrentFrame() const;

protected:
	bool NextJob(RefCountingPtr<AsyncNode>& JobOut);

	void SetThreadDescription();

protected:
	int8 Index;
	int8 TaskIndex = -1;
	ThreadType Type;
	JobScheduler* Owner;
	std::string Name;
	std::jthread ThreadImpl;
	std::atomic<bool> IsRunning{false};
	std::binary_semaphore IsReady{0};
	std::atomic<uint64> CurrentFrame;
	std::deque<RefCountingPtr<JobNode>> LocalQueue;
	std::deque<RefCountingPtr<AsyncTaskNodeBase>> LocalTaskQueue;
	std::mutex LocalQueueMutex;
};
}
