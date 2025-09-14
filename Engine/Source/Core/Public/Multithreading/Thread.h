#pragma once
#include <thread>
#include <deque>
#include <mutex>
#include <semaphore>

#include "JobNode.h"
#include "Templates/RefCounters.h"

namespace LE
{
class JobScheduler;
}

namespace LE
{
class Thread : RefCountableBase
{
public:
	Thread(uint8 InIndex, std::string InName, JobScheduler* InOwner)
		: Index(InIndex)
		  , Owner(InOwner)
		  , Name(std::move(InName))
	{
	}

	Thread(const Thread&) = delete;

	Thread(Thread&& Other) noexcept
	{
		Index = Other.Index;
		Owner = Other.Owner;
		Name = Other.Name;
		std::swap(ThreadImpl, Other.ThreadImpl);
		std::swap(LocalQueue, Other.LocalQueue);
	}

	Thread& operator=(const Thread&) = delete;

	Thread& operator=(Thread&& Other) noexcept
	{
		std::swap(Index, Other.Index);
		std::swap(Owner, Other.Owner);
		std::swap(Name, Other.Name);
		std::swap(ThreadImpl, Other.ThreadImpl);
		std::swap(LocalQueue, Other.LocalQueue);
		return *this;
	}

	~Thread() override
	{
		if (ThreadImpl.joinable())
		{
			ThreadImpl.join();
		}
	}

	void Start();
	void Stop();

	void Main();

	bool TryPushJob(RefCountingPtr<JobNode> JobToAdd);
	void PushJob(RefCountingPtr<JobNode> JobToAdd);
	bool TryStealJob(RefCountingPtr<JobNode>& JobOut);

private:
	bool NextJob(RefCountingPtr<JobNode>& JobOut);

	void SetThreadDescription();

private:
	uint8 Index;
	JobScheduler* Owner;
	std::string Name;
	std::thread ThreadImpl;
	std::atomic<bool> IsRunning{false};
	std::binary_semaphore IsReady{0};
	std::deque<RefCountingPtr<JobNode>> LocalQueue;
	std::mutex LocalQueueMutex;
};
}
