#include "Multithreading/JobNode.h"

#include "Multithreading/JobScheduler.h"

#include "Time/Clock.h"

namespace LE
{
void JobNode::Execute()
{
	Function(Clock::GetElapsedSeconds());
	OnCompleted();
}

void JobNode::IncrementDependencyCounter()
{
	JobsTillReady.fetch_add(1, std::memory_order_release);
}

void JobNode::DecrementDependencyCounter()
{
	uint32 newValue = JobsTillReady.fetch_sub(1, std::memory_order_acq_rel) - 1;
	LE_ASSERT(newValue < 3000)
	if (newValue == 0)
	{
		if (Owner)
		{
			Owner->OnJobBecameAvailable(this);
		}
	}
}

void JobNode::OnCompleted()
{
	for (auto& dependentJob : DependentJobs)
	{
		dependentJob->DecrementDependencyCounter();
	}
	JobsTillReady.store(DefaultDependencies, std::memory_order_release);
	if (Owner)
	{
		Owner->OnJobFinished();
	}
}
}
