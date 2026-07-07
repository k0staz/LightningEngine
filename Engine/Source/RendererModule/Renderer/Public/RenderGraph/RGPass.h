#pragma once
#include <algorithm>

#include "RGResource.h"
#include "RHIResources.h"
#include "Memory/MemoryArena.h"

namespace LE::Renderer
{
class RenderGraph;

struct RGAccess
{
    RGTexture Texture;
    RGState State = RGState::None;
};

struct RGAttachment
{
    RGTexture Texture;
    RHI::RHILoadOp LoadOp = RHI::RHILoadOp::Load;
    RHI::RHIStoreOp StoreOp = RHI::RHIStoreOp::Store;

    union ClearValue
    {
        LinearColor ClearColor = {0.0f, 0.0f, 0.0f, 0.0f};

        struct
        {
            float ClearDepth = 1.0f;
            uint32 ClearStencil = 0;
        } DepthStencil;
    } ClearValue = {};

    bool IsValid() const
    {
        return Texture.IsValid();
    }
};

class RGPassBase
{
public:
    RGPassBase(const char* InName, MemoryArena& Arena)
        : Name(InName)
          , ReadAccesses(ArenaAllocator<RGAccess>(Arena))
          , WriteAccesses(ArenaAllocator<RGAccess>(Arena))
          , ColorAttachments(ArenaAllocator<RGAttachment>(Arena))
          , ColorAttachmentsDesc(ArenaAllocator<RHI::RHIRenderingAttachmentDesc>(Arena))
    {
    }

    virtual ~RGPassBase() = default;

    virtual void Execute(RHI::RHICommandList& CommandList, RenderGraph& RenderGraph) = 0;

    const char* Name;

    std::vector<RGAccess, ArenaAllocator<RGAccess>> ReadAccesses;
    std::vector<RGAccess, ArenaAllocator<RGAccess>> WriteAccesses;
    std::vector<RGAttachment, ArenaAllocator<RGAttachment>> ColorAttachments;
    RGAttachment DepthAttachment = {};

    std::vector<RHI::RHIRenderingAttachmentDesc, ArenaAllocator<RHI::RHIRenderingAttachmentDesc>> ColorAttachmentsDesc;
    RHI::RHIRenderingAttachmentDesc DepthAttachmentDesc;

    RHI::RHIRenderingDesc RenderingDesc = {};
    RHI::RHIDependencyDesc CompiledDependency = {};
};

template <typename ExecuteFunction>
class RGPass : public RGPassBase
{
public:
    explicit RGPass(const char* InName, ExecuteFunction&& InExecuteFunc, MemoryArena& Arena)
        : RGPassBase(InName, Arena)
          , ExecuteFunc(std::forward<ExecuteFunction>(InExecuteFunc))
    {
    }

    void Execute(RHI::RHICommandList& CommandList, RenderGraph& RGraph) override
    {
        ExecuteFunc(CommandList, RGraph);
    }

private:
    ExecuteFunction ExecuteFunc;
};
}
