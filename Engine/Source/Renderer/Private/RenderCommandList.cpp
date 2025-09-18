#include "RenderCommandList.h"

#include <deque>
#include <mutex>

#include "DynamicRHI.h"

namespace LE::Renderer
{

// TODO: This mess down here should be burned down, and its needs to be build anew
static RenderCommandList gRenderCommands;
static RenderCommandList gRunningRenderCommands;
static std::mutex gRenderCommandsMutex;
static std::atomic_bool IsRunning;
static std::condition_variable IsRunningCV;

RenderCommandList& RenderCommandList::Get()
{
	return gRenderCommands;
}

void RenderCommandList::StartFrameRenderCommandList()
{
	std::unique_lock lock(gRenderCommandsMutex);
	IsRunningCV.wait(lock, [] {return IsRunning.load(std::memory_order_acquire) == false; });

	gRunningRenderCommands = gRenderCommands;
	gRenderCommands.Clear();
}

void RenderCommandList::StartExecution()
{
	{
		std::unique_lock lock(gRenderCommandsMutex);
		IsRunning.store(true, std::memory_order_relaxed);
		IsRunningCV.notify_all();
	}
	gRunningRenderCommands.Execute();
	{
		std::unique_lock lock(gRenderCommandsMutex);
		IsRunning.store(false, std::memory_order_relaxed);
		IsRunningCV.notify_all();
	}
}

void RenderCommandList::Execute()
{
	IsExecuting = true;
	for (auto& renderCommand : RenderCommands)
	{
		renderCommand.Execute(*this);
	}

	RenderCommands.clear();
	IsExecuting = false;
}

void RenderCommandList::Clear()
{
	RenderCommands.clear();
	IsExecuting = false;
	ScratchShaderParametersCollection.Reset();
}

RefCountingPtr<RHI::RHIBuffer> RenderCommandList::CreateBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags, uint32 Stride,
                                                               RHI::RHIResourceCreateInfo& CreateInfo)
{
	RHI::RHIBufferDesc bufferDesc{Size, Stride, UsageFlags};

	RefCountingPtr<RHI::RHIBuffer> buffer = RHI::gDynamicRHI->RHICreateBuffer(bufferDesc, CreateInfo);
	return buffer;
}

void RenderCommandList::UpdateConstantBuffer(RHI::RHIConstantBuffer* ConstantBuffer, const void* Value)
{
	RHI::gDynamicRHI->RHIUpdateConstantBuffer(*this, ConstantBuffer, Value);
}

RefCountingPtr<RHI::RHIBuffer> RenderCommandList::CreateVertexBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags,
                                                                     RHI::RHIResourceCreateInfo& CreateInfo)
{
	return CreateBuffer(Size, UsageFlags | RHI::BUF_Vertex, 0, CreateInfo);
}

RefCountingPtr<RHI::RHIBuffer> RenderCommandList::CreateIndexBuffer(uint32 Stride, uint32 Size, RHI::BufferUsageFlags UsageFlags,
	RHI::RHIResourceCreateInfo& CreateInfo)
{
	return CreateBuffer(Size, UsageFlags, Stride, CreateInfo);
}

RefCountingPtr<RHI::RHIReadView> RenderCommandList::CreateReadView(RHI::RHIBuffer* Buffer,
                                                                   const RHI::RHIViewDescription::BufferReadViewInfo::Initializer& ViewDesc)
{
	return RHI::gDynamicRHI->RHICreateReadView(*this, Buffer, ViewDesc);
}

void RenderCommandList::BeginDrawingViewport(RHI::RHIViewport* Viewport)
{
	RHI::gDynamicRHI->RHIBeginDrawingViewport(Viewport);
}

void RenderCommandList::EndDrawingViewport(RHI::RHIViewport* Viewport)
{
	RHI::gDynamicRHI->RHIEndDrawingViewport(Viewport);
}

void RenderCommandList::EnqueueLambdaCommand(const RenderCommand& LambdaCommand)
{
	if (IsExecuting)
	{
		LambdaCommand(*this);
	}
	else
	{
		RenderCommands.emplace_back(LambdaCommand);
	}
}

void RenderCommandList::SetGraphicsPSO(RHI::RHIPipelineStateObject* RHIPipelineStateObject, uint32 StencilRef)
{
	GetContext().RHISetPSO(RHIPipelineStateObject, StencilRef);
}

void RenderCommandList::DrawIndexedPrimitive(RHI::RHIBuffer* IndexBuffer, uint32 BaseVertexIndex, uint32 StartIndex, uint32 PrimitiveCount)
{
	GetContext().RHIDrawIndexedPrimitive(IndexBuffer, BaseVertexIndex, StartIndex, PrimitiveCount);
}

void RenderCommandList::SetShaderParametersCollection(RHI::RHIShader* Shader, RHI::RHIShaderParametersCollection& ParametersCollection)
{
	GetContext().RHISetShaderParameters(Shader, ParametersCollection.ParametersData, ParametersCollection.Parameters, ParametersCollection.ResourceParameters);
	ParametersCollection.Reset();
}

RHI::RHIContext& RenderCommandList::GetContext()
{
	return *RHI::gDynamicRHI->RHIGetContext();
}
}
