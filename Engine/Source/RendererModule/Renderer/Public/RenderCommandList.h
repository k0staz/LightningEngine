#pragma once
#include <semaphore>

#include "RHIResources.h"


namespace LE::Renderer
{
class RenderCommandList;

using RenderCommand = std::function<void(RenderCommandList& CmdList)>;

class RenderCommandWrapper
{
public:
	explicit RenderCommandWrapper(RenderCommand InCommand)
		: Command(std::move(InCommand))
	{}

	void Execute(RenderCommandList& CmdList) const
	{
		Command(CmdList);
	}

private:
	RenderCommand Command;
};

class RenderCommandList
{
public:
	static RenderCommandList& Get(); // Should be called on GT

	RenderCommandList();

	void Initialize(int8 WorkerThreadNum);
	void EnqueueLambdaCommand(const RenderCommand LambdaCommand);

	void FinalizeFrame(); // Joins commands from worker thread and puts them into reading list
	void ExecuteRenderCommands(); // Should be called from render frame

private:
	std::vector<RenderCommandWrapper> ReadRenderCommands; // Those are executed on the render thread
	std::vector<std::vector<RenderCommandWrapper>> WriteRenderCommands; // Those are where worker threads write

	std::binary_semaphore RenderThreadFinished{0};
};
}
