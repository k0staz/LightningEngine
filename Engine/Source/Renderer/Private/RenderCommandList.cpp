#include "RenderCommandList.h"

#include "DynamicRHI.h"
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

void RenderCommandList::EnqueueLambdaCommand(const RenderCommand& LambdaCommand)
{
	if (Thread::IsRenderThread())
	{
		LambdaCommand(*this);
	}
	else
	{
		const int8 workerThreadIdx = Thread::IsMainThread()? static_cast<int8>(0) : Thread::GetWorkerThreadIndex();
		LE_ASSERT_DESC(workerThreadIdx >= 0, "Trying to enqueue render command from non-working thread")
		WriteRenderCommands[workerThreadIdx].emplace_back(LambdaCommand);
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

void RenderCommandList::Render_ExecuteFrame()
{
	for (RenderCommandWrapper& command : ReadRenderCommands)
	{
		command.Execute(*this);
	}

	ReadRenderCommands.clear();
	RenderThreadFinished.release();
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
	GetContext().RHISetShaderParameters(Shader, ParametersCollection.ParametersData, ParametersCollection.Parameters,
	                                    ParametersCollection.ResourceParameters);
	ParametersCollection.Reset();
}

RHI::RHIContext& RenderCommandList::GetContext()
{
	return *RHI::gDynamicRHI->RHIGetContext();
}
}
