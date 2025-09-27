#pragma once
#include <semaphore>

#include "RHIContext.h"
#include "RHIResources.h"
#include "RHIShaderParameters.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
class RenderCommandList;

using RenderCommand = std::function<void(RenderCommandList& CmdList)>;

class RenderCommandWrapper
{
public:
	explicit RenderCommandWrapper(RenderCommand InCommand)
		: Command(std::move(InCommand))
	{
	}

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
	void EnqueueLambdaCommand(const RenderCommand& LambdaCommand);

	void FinalizeFrame(); // Joins commands from worker thread and puts them into reading list
	void Render_ExecuteFrame(); // Should be called from render frame

	RefCountingPtr<RHI::RHIBuffer> CreateBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags, uint32 Stride,
	                                            RHI::RHIResourceCreateInfo& CreateInfo);

	void UpdateConstantBuffer(RHI::RHIConstantBuffer* ConstantBuffer, const void* Value);

	RefCountingPtr<RHI::RHIBuffer> CreateVertexBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags, RHI::RHIResourceCreateInfo& CreateInfo);
	RefCountingPtr<RHI::RHIBuffer> CreateIndexBuffer(uint32 Stride, uint32 Size, RHI::BufferUsageFlags UsageFlags, RHI::RHIResourceCreateInfo& CreateInfo);

	RefCountingPtr<RHI::RHIReadView> CreateReadView(RHI::RHIBuffer* Buffer,
	                                                const RHI::RHIViewDescription::BufferReadViewInfo::Initializer& ViewDesc);

	void BeginDrawingViewport(RHI::RHIViewport* Viewport);
	void EndDrawingViewport(RHI::RHIViewport* Viewport);

	void SetGraphicsPSO(RHI::RHIPipelineStateObject* RHIPipelineStateObject, uint32 StencilRef);
	void DrawIndexedPrimitive(RHI::RHIBuffer* IndexBuffer, uint32 BaseVertexIndex, uint32 StartIndex, uint32 PrimitiveCount);

	void SetShaderParametersCollection(RHI::RHIShader* Shader, RHI::RHIShaderParametersCollection& ParametersCollection);

	RHI::RHIContext& GetContext();

	RHI::RHIShaderParametersCollection& GetScratchShaderParametersCollection()
	{
		if (ScratchShaderParametersCollection.HasAnyParameter())
		{
			LE_ASSERT_DESC(false, "Shader parameters were not committed")
			ScratchShaderParametersCollection.Reset();
		}

		return ScratchShaderParametersCollection;
	}

protected:
	RHI::RHIShaderParametersCollection ScratchShaderParametersCollection;

private:
	std::vector<RenderCommandWrapper> ReadRenderCommands; // Those are executed on the render thread
	std::vector<std::vector<RenderCommandWrapper>> WriteRenderCommands; // Those are where worker threads write

	std::binary_semaphore RenderThreadFinished{0};
};
}
