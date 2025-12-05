#pragma once
#include <mutex>
#include <unordered_set>

#include "AsyncNode.h"
#include "Misc/Delegate.h"
#include "Templates/RefCounters.h"

namespace LE
{
class JobScheduler;

class AsyncTaskNodeBase : public AsyncNode
{
	friend JobScheduler;

public:
	AsyncTaskNodeBase(std::string_view Name, JobScheduler* Owner)
		: Owner(Owner),
		  TaskName(Name)
	{
	}

	void Finalize();

	uint32 GetCurrentRemainingTaskCount() const;
	bool IsReady() const;
	std::string_view GetName() const;
	void AddDependentTask(AsyncTaskNodeBase* Task);
	void RemoveDependentTask(AsyncTaskNodeBase* Task);

protected:
	void IncrementDependencyCounter();
	void DecrementDependencyCounter();
	void OnCompleted();

private:
	JobScheduler* Owner = nullptr;
	std::unordered_set<RefCountingPtr<AsyncTaskNodeBase>> DependentTasks;
	std::string_view TaskName;
	std::atomic_uint TasksTillReady;
	std::atomic_bool IsFinalized = false;
	bool IsFinished = false;
	mutable std::mutex IsFinishedMutex;
	mutable std::mutex DependentTaskMutex;
};

template <typename TaskBody>
class AsyncTaskNode : public AsyncTaskNodeBase
{
public:
	AsyncTaskNode(std::string_view Name, JobScheduler* Owner, std::function<TaskBody> Function)
		: AsyncTaskNodeBase(Name, Owner),
		  TaskFunction(Function)
	{
	}

	void Execute() override
	{
		TaskFunction();
		OnCompleted();
	}

private:
	std::function<TaskBody> TaskFunction;
};
}
