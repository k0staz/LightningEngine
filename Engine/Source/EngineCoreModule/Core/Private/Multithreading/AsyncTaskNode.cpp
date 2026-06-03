#include "Multithreading/AsyncTaskNode.h"

#include "Multithreading/JobScheduler.h"

namespace LE
{
void AsyncTaskNodeBase::Finalize()
{
	{
		std::lock_guard finishedGuard(IsFinishedMutex);
		IsFinished = false;
	}
	
	IsFinalized.store(true, std::memory_order_release);
	if (Owner)
	{
		Owner->OnTaskFinalized(this);
	}
}

uint32 AsyncTaskNodeBase::GetCurrentRemainingTaskCount() const
{
	return TasksTillReady.load(std::memory_order_acquire);
}

bool AsyncTaskNodeBase::IsReady() const
{
	return IsFinalized.load(std::memory_order_acquire) && GetCurrentRemainingTaskCount() == 0;
}

std::string_view AsyncTaskNodeBase::GetName() const
{
	return TaskName;
}

void AsyncTaskNodeBase::AddDependentTask(AsyncTaskNodeBase* Task)
{
	// We need to maintain this mutex for the whole duration, to avoid missing on Finished
	std::lock_guard finishedGuard(IsFinishedMutex);
	if(IsFinished)
	{
		return;
	}
	
	std::lock_guard guard(DependentTaskMutex);
	if (!Task || DependentTasks.contains(Task))
	{
		return;
	}

	DependentTasks.emplace(Task);
	Task->IncrementDependencyCounter();
}

void AsyncTaskNodeBase::RemoveDependentTask(AsyncTaskNodeBase* Task)
{
	std::lock_guard guard(DependentTaskMutex);
	if (!Task || !DependentTasks.contains(Task))
	{
		return;
	}

	DependentTasks.erase(Task);
	Task->DecrementDependencyCounter();
}

void AsyncTaskNodeBase::IncrementDependencyCounter()
{
	TasksTillReady.fetch_add(1, std::memory_order_release);
}

void AsyncTaskNodeBase::DecrementDependencyCounter()
{
	uint32 newValue = TasksTillReady.fetch_sub(1, std::memory_order_acq_rel) - 1;
	if (newValue == 0)
	{
		if (Owner)
		{
			Owner->OnTaskBecameAvailable(this);
		}
	}
}

void AsyncTaskNodeBase::OnCompleted()
{
	std::lock_guard finishedGuard(IsFinishedMutex);
	IsFinished = true;
	for (auto& dependentTask: DependentTasks)
	{
		dependentTask->DecrementDependencyCounter();
	}
}
}
