#include "RenderCommandList.h"

#include "Multithreading/Thread.h"

namespace
{
LE::Renderer::RenderCommandList gRenderCommands;
}

namespace LE::Renderer
{
RenderCommandList& RenderCommandList::Get()
{
	return gRenderCommands;
}

RenderCommandList::RenderCommandList()
{
	WriteRenderCommands.resize(1);
}

void RenderCommandList::Initialize(int8 WorkerThreadNum)
{
	WriteRenderCommands.resize(WorkerThreadNum + 1);
	RenderThreadFinished.release();
}

void RenderCommandList::EnqueueLambdaCommand(const RenderCommand LambdaCommand)
{
	if (Thread::IsRenderThread())
	{
		LambdaCommand(*this);
	}
	else
	{
		const int8 workerThreadIdx = Thread::IsMainThread()? static_cast<int8>(WriteRenderCommands.size() - 1) : Thread::GetWorkerThreadIndex();
		LE_ASSERT_DESC(workerThreadIdx >= 0, "Trying to enqueue render command from non-working thread")
		WriteRenderCommands[workerThreadIdx].emplace_back(std::move(LambdaCommand));
	}
}

void RenderCommandList::FinalizeFrame()
{
	std::vector<RenderCommandWrapper> finalRenderCommandList;

	size_t finalSize = 0;
	for (const std::vector<RenderCommandWrapper>& commandList : WriteRenderCommands)
	{
		finalSize += commandList.size();
	}

	finalRenderCommandList.reserve(finalSize);
	for (std::vector<RenderCommandWrapper>& commandList : WriteRenderCommands)
	{
		finalRenderCommandList.insert(finalRenderCommandList.end(), commandList.begin(), commandList.end());
		commandList.clear();
	}

	// Wait for the render frame to finish
	RenderThreadFinished.acquire();

	ReadRenderCommands.reserve(finalSize);
	ReadRenderCommands.insert(ReadRenderCommands.end(), finalRenderCommandList.begin(), finalRenderCommandList.end());
}

void RenderCommandList::ExecuteRenderCommands()
{
	for (RenderCommandWrapper& command : ReadRenderCommands)
	{
		command.Execute(*this);
	}

	ReadRenderCommands.clear();
	RenderThreadFinished.release();
}
}
