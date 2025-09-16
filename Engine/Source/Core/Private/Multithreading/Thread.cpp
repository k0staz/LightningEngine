#include "Multithreading/Thread.h"

#include "Multithreading/JobScheduler.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace LE
{
void Thread::Start()
{
	IsRunning.store(true, std::memory_order_relaxed);
	ThreadImpl = std::thread([this] {Main(); });
}

void Thread::Stop()
{
	IsRunning.store(false, std::memory_order_relaxed);
	IsReady.release();
}

void Thread::Main()
{
	SetThreadDescription();

	while (IsRunning.load(std::memory_order_relaxed))
	{
		IsReady.acquire();

		RefCountingPtr<JobNode> currentJob = nullptr;
		while (NextJob(currentJob))
		{
			currentJob->Execute();
		}
	}
}

bool Thread::TryPushJob(RefCountingPtr<JobNode> JobToAdd)
{
	std::unique_lock lock(LocalQueueMutex, std::try_to_lock);
	if (!lock)
	{
		return false;
	}

	LocalQueue.push_front(JobToAdd);
	IsReady.release();
	return true;
}

void Thread::PushJob(RefCountingPtr<JobNode> JobToAdd)
{
	std::unique_lock lock(LocalQueueMutex);
	LocalQueue.push_front(JobToAdd);
	IsReady.release();
}

bool Thread::TryStealJob(RefCountingPtr<JobNode>& JobOut)
{
	std::unique_lock lock(LocalQueueMutex, std::try_to_lock);
	if (!lock)
	{
		return false;
	}

	if (LocalQueue.empty())
	{
		return false;
	}

	JobOut = LocalQueue.front();
	LocalQueue.pop_front();

	return true;
}

bool Thread::NextJob(RefCountingPtr<JobNode>& JobOut)
{
	{
		std::unique_lock lock(LocalQueueMutex);
		if (lock && !LocalQueue.empty())
		{
			JobOut = LocalQueue.back();
			LocalQueue.pop_front();
			return true;
		}
	}

	return Owner->TryStealJobFromThread(Index, JobOut);
}

void Thread::SetThreadDescription()
{
	LE_INFO("Thread {} started", Name);
#if PLATFORM_WINDOWS
	using SetThreadDescriptionFunc = HRESULT(WINAPI*)(HANDLE hThread, PCWSTR lpThreadDescription);
	static SetThreadDescriptionFunc SetThreadDescription = reinterpret_cast<SetThreadDescriptionFunc>(GetProcAddress(GetModuleHandleW(L"Kernel32.dll"), "SetThreadDescription"));
	if (SetThreadDescription)
	{
		wchar_t name_buffer[64] = { 0 };
		if (MultiByteToWideChar(CP_UTF8, 0, Name.c_str(), -1, name_buffer, sizeof(name_buffer) / sizeof(wchar_t) - 1) == 0)
			return;

		SetThreadDescription(GetCurrentThread(), name_buffer);
	}
#endif
}
}
