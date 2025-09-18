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
enum class ThreadType : uint8_t
{
	Worker,
	Render
};

class Thread : public RefCountableBase
{
public:
	Thread(uint8 InIndex, std::string InName, ThreadType InType, JobScheduler* InOwner)
		: Index(InIndex)
		  , Type(InType)
		  , Owner(InOwner)
		  , Name(std::move(InName))
	{
	}

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

	~Thread() override
	{
		if (ThreadImpl.joinable())
		{
			ThreadImpl.join();
		}
	}

	ThreadType GetType() const
	{
		return Type;
	}

	void Start();
	void Stop();

	void Main();

	bool TryPushJob(RefCountingPtr<JobNode> JobToAdd);
	void PushJob(RefCountingPtr<JobNode> JobToAdd);
	bool TryStealJob(RefCountingPtr<JobNode>& JobOut);

	void IncrementFrameCounter();
	uint64 GetCurrentFrame() const;

protected:
	bool NextJob(RefCountingPtr<JobNode>& JobOut);

	void SetThreadDescription();

protected:
	uint8 Index;
	ThreadType Type;
	JobScheduler* Owner;
	std::string Name;
	std::thread ThreadImpl;
	std::atomic<bool> IsRunning{false};
	std::binary_semaphore IsReady{0};
	std::atomic<uint64> CurrentFrame;
	std::deque<RefCountingPtr<JobNode>> LocalQueue;
	std::mutex LocalQueueMutex;
};
}
