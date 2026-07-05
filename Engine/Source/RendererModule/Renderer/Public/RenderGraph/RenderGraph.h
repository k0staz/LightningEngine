#pragma once
#include "RGPass.h"
#include "Memory/MemoryArena.h"
#include "Service/ServiceBase.h"
#include <utility>

namespace LE::Renderer
{
class RenderGraph;

class RGBuilder : public NonCopyable
{
public:
    explicit RGBuilder(RGPassBase& InPass)
        : Pass(InPass)
    {
    }

    void SetColorAttachment(uint32 Slot, RGTexture Texture, RHI::RHILoadOp LoadOp, RHI::RHIStoreOp StoreOp, LinearColor ClearValue);
    void SetDepthAttachment(RGTexture Texture, RHI::RHILoadOp LoadOp, RHI::RHIStoreOp StoreOp, float ClearValue, uint32 ClearStencil);
    void Read(RGTexture Texture, RGState State);
    void Write(RGTexture Texture, RGState State);

private:
    RGPassBase& Pass;
};

class RenderGraph : public ServiceBase
{
public:
    void Initialize() override;
    void Shutdown() override;

    template <typename SetupFunction, typename ExecuteFunction>
    void AddPass(const char* Name, SetupFunction&& SetupFunc, ExecuteFunction&& ExecuteFunc)
    {
        Passes.emplace_back(Arena.Create<RGPass<ExecuteFunction>>(Name, std::forward<ExecuteFunction>(ExecuteFunc), Arena));

        RGBuilder builder(*Passes.back());
        SetupFunc(builder);
    }

    void Compile();
    void Execute();
    void Present(RHI::RHIWindow* Window);
    void Reset();

    RGTexture ImportSwapchainImage(RHI::RHIWindow* Window);
    RGTexture ImportDepthImage(RHI::RHIWindow* Window);

    [[nodiscard]] uint32 GetSwapchainIndex() const { return SwapchainIndex; }

private:
    RGTexture RegisterTexture(RGTextureEntry&& Entry);

    void OrderPasses();
    void ProcessPasses();

    void BuildRenderingDesc(RGPassBase& Pass);

private:
    std::vector<RGPassBase*> Passes;
    std::vector<RGPassBase*> OrderedPasses;
    std::vector<RGTextureEntry*> TextureResources;
    RHI::RHIDependencyDesc CompiledExportDeps;
    uint32 SwapchainIndex = 0;
    MemoryArena Arena;
};
}

namespace LE
{
REGISTER_SERVICE_TYPE(Renderer::RenderGraph, "RenderGraph")
}
