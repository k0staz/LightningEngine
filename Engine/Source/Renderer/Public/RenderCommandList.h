#pragma once
#include "RHIContext.h"
#include "RHIResources.h"
#include "RHIShaderParameters.h"
#include "Templates/RefCounters.h"


namespace LE::Renderer
{
class RenderCommandList;

using RenderCommand = std::function<void(RenderCommandList& CmdList)>;

class ICommandWrapper
{
public:
	void Execute(RenderCommandList& CmdList)
	{
		Call(CmdList);
	}

protected:
	virtual void Call(RenderCommandList& CmdList) = 0;
};

class CommandWrapper final : public ICommandWrapper
{
public:
	explicit CommandWrapper(RenderCommand InCommand)
		: Command(std::move(InCommand))
	{
	}

protected:
	virtual void Call(RenderCommandList& CmdList) override
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
	static void StartFrameRenderCommandList(); // Should be called on GT
	static void StartExecution(); // Should be called on Render Thread

	void Execute();
	void Clear();

	RefCountingPtr<RHI::RHIBuffer> CreateBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags, uint32 Stride,
	                                            RHI::RHIResourceCreateInfo& CreateInfo);

	void UpdateConstantBuffer(RHI::RHIConstantBuffer* ConstantBuffer, const void* Value);

	RefCountingPtr<RHI::RHIBuffer> CreateVertexBuffer(uint32 Size, RHI::BufferUsageFlags UsageFlags, RHI::RHIResourceCreateInfo& CreateInfo);
	RefCountingPtr<RHI::RHIBuffer> CreateIndexBuffer(uint32 Stride, uint32 Size, RHI::BufferUsageFlags UsageFlags, RHI::RHIResourceCreateInfo& CreateInfo);

	RefCountingPtr<RHI::RHIReadView> CreateReadView(RHI::RHIBuffer* Buffer,
	                                                const RHI::RHIViewDescription::BufferReadViewInfo::Initializer& ViewDesc);

	void BeginDrawingViewport(RHI::RHIViewport* Viewport);
	void EndDrawingViewport(RHI::RHIViewport* Viewport);

	void EnqueueLambdaCommand(const RenderCommand& LambdaCommand);

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
	Array<CommandWrapper> RenderCommands;
	bool IsExecuting = false;
};
}
