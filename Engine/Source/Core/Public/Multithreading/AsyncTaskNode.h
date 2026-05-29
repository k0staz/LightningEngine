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

template <typename TaskBody, typename... Args>
class AsyncTaskNode : public AsyncTaskNodeBase
{
public:
	AsyncTaskNode(std::string_view Name, JobScheduler* Owner, TaskBody Function, Args&&... FunctionArgs)
		: AsyncTaskNodeBase(Name, Owner),
		  TaskFunction(std::forward<TaskBody>(Function)),
		  Parameters(std::forward<Args>(FunctionArgs)...)
	{
	}

	void Execute() override
	{
		std::apply(std::move(TaskFunction), std::move(Parameters));
		OnCompleted();
	}

private:
	TaskBody TaskFunction;
	std::tuple<Args...> Parameters;
};

namespace MultithreadingUtils
{
template <typename TaskBody, typename... Args>
RefCountingPtr<AsyncTaskNodeBase> MakeTask(std::string_view Name, JobScheduler* Owner, TaskBody Function, Args&&... FunctionArgs)
{
	return new AsyncTaskNode<std::decay_t<TaskBody>, std::decay_t<Args>...>(Name, Owner, std::forward<TaskBody>(Function), std::forward<Args>(FunctionArgs)...);
}
}
}
