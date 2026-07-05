#include <utility>

#include "RenderGraph/RenderGraph.h"

#include "RHIDevice.h"
#include "RenderResourceManager/RenderResourceManager.h"

namespace LE::Renderer
{
void RGBuilder::SetColorAttachment(uint32 Slot, RGTexture Texture, RHI::RHILoadOp LoadOp, RHI::RHIStoreOp StoreOp, LinearColor ClearValue)
{
    RGAttachment attachment{Texture, LoadOp, StoreOp, {}};
    attachment.ClearValue.ClearColor = ClearValue;
    if (Pass.ColorAttachments.size() <= Slot)
    {
        Pass.ColorAttachments.resize(Slot + 1);
    }
    Pass.ColorAttachments[Slot] = attachment;
    Pass.ColorAttachmentsDesc.resize(Slot + 1);
}

void RGBuilder::SetDepthAttachment(RGTexture Texture, RHI::RHILoadOp LoadOp, RHI::RHIStoreOp StoreOp, float ClearValue, uint32 ClearStencil)
{
    RGAttachment attachment{Texture, LoadOp, StoreOp, {}};
    attachment.ClearValue.DepthStencil.ClearDepth = ClearValue;
    attachment.ClearValue.DepthStencil.ClearStencil = ClearStencil;
    Pass.DepthAttachment = attachment;
}

void RGBuilder::Read(RGTexture Texture, RGState State)
{
    Pass.ReadAccesses.emplace_back(Texture, State);
}

void RGBuilder::Write(RGTexture Texture, RGState State)
{
    Pass.WriteAccesses.emplace_back(Texture, State);
}

void RenderGraph::Initialize()
{
}

void RenderGraph::Shutdown()
{
    Reset();
}

void RenderGraph::Compile()
{
    ZoneScopedN("RenderGraph::Compile");
    OrderPasses();
    ProcessPasses();
}

void RenderGraph::Execute()
{
    ZoneScopedN("RenderGraph::Execute");
    RHI::RHIDevice* device = RHI::RHIDevice::Get();

    std::vector<RefCountingPtr<RHI::RHICommandList>> commandLists;
    commandLists.reserve(OrderedPasses.size() + 1);
    {
        auto& renderResourceManager = GetServiceRegistry().GetService<RenderResourceManager>();
        if (renderResourceManager.HasPendingBarriers())
        {
            RefCountingPtr<RHI::RHICommandList> cmd = device->CreateCommandList(RHI::RHICommandListType::Graphics);
            cmd->BeginRecording();
            renderResourceManager.EnqueuePendingBarriers(cmd);
            cmd->EndRecording();
            commandLists.emplace_back(cmd);
        }
    }

    for (auto& Pass : OrderedPasses)
    {
        RefCountingPtr<RHI::RHICommandList> cmd = device->CreateCommandList(RHI::RHICommandListType::Graphics);
        cmd->BeginRecording();

        cmd->PipelineBarrier(Pass->CompiledDependency);
        cmd->BeginRendering(Pass->RenderingDesc);

        const Vector2U defaultExt = Pass->RenderingDesc.RectangleExtent;
        cmd->SetViewport({.Width = (float)defaultExt.X, .Height = (float)defaultExt.Y, .MinDepth = 0, .MaxDepth = 1});
        cmd->SetScissor(defaultExt);

        Pass->Execute(*cmd, *this);

        cmd->EndRendering();
        cmd->EndRecording();

        commandLists.emplace_back(cmd);
    }

    {
        RefCountingPtr<RHI::RHICommandList> cmd = device->CreateCommandList(RHI::RHICommandListType::Graphics);
        cmd->BeginRecording();
        cmd->PipelineBarrier(CompiledExportDeps);
        cmd->EndRecording();
        commandLists.emplace_back(cmd);
    }

    device->SubmitCommandList(RHI::RHICommandListType::Graphics, commandLists, SwapchainIndex);
}

void RenderGraph::Present(RHI::RHIWindow* Window)
{
    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    device->Present(Window, SwapchainIndex);
}

void RenderGraph::Reset()
{
    Arena.Reset();
    Passes.clear();
    OrderedPasses.clear();
    TextureResources.clear();
    CompiledExportDeps = {};
    SwapchainIndex = 0;
}

RGTexture RenderGraph::ImportSwapchainImage(RHI::RHIWindow* Window)
{
    RHI::RHIDevice* device = RHI::RHIDevice::Get();
    if (!device->GetNextSwapchainImageIndex(Window, SwapchainIndex))
    {
        LE_ASSERT_DESC(false, "Failed to get swapchain image index");
        return {};
    }

    RGTextureEntry entry;
    entry.Image = Window->GetSwapchainImage(SwapchainIndex);
    entry.View = Window->GetSwapchainImageView(SwapchainIndex);
    entry.CurrentState = ToPhysical(RGState::None);
    entry.FinalState = RGState::Present;
    entry.IsImported = true;
    entry.Range = {RHI::RHIImageAspectFlags::Color, 0, 1, 0, 1};
    return RegisterTexture(std::move(entry));
}

RGTexture RenderGraph::ImportDepthImage(RHI::RHIWindow* Window)
{
    RGTextureEntry entry;
    entry.Image = Window->GetDepthImage();
    entry.View = Window->GetDepthImageView();
    entry.CurrentState = ToPhysical(RGState::None);
    entry.FinalState = RGState::None;
    entry.IsImported = true;
    entry.Range = {RHI::RHIImageAspectFlags::Depth | RHI::RHIImageAspectFlags::Stencil, 0, 1, 0, 1};
    return RegisterTexture(std::move(entry));
}

RGTexture RenderGraph::RegisterTexture(RGTextureEntry&& Entry)
{
    TextureResources.emplace_back(Arena.Create<RGTextureEntry>(std::forward<RGTextureEntry>(Entry)));
    return {static_cast<uint32>(TextureResources.size() - 1)};
}

void RenderGraph::OrderPasses()
{
    OrderedPasses = Passes;
}

void RenderGraph::ProcessPasses()
{
    auto transitionState = [&](RGTextureEntry& Entry, RGState ReqState, RHI::RHIDependencyDesc& dependency)
    {
        const RGPhysicalState reqPhysState = ToPhysical(ReqState);
        if (reqPhysState.Layout != Entry.CurrentState.Layout ||
            (reqPhysState.Access & RHI::RHIAccessFlags::Write) ||
            Entry.CurrentState.Layout == RHI::RHIImageLayout::None)
        {
            RHI::RHIImageMemoryBarrierDesc& barrier = dependency.ImageMemoryBarriers.emplace_back();
            barrier.Image = Entry.Image;
            barrier.OldLayout = Entry.CurrentState.Layout;
            barrier.NewLayout = reqPhysState.Layout;
            barrier.SrcStageFlags = Entry.CurrentState.Stage;
            barrier.DstStageFlags = reqPhysState.Stage;
            barrier.SrcAccessFlags = Entry.CurrentState.Access;
            barrier.DstAccessFlags = reqPhysState.Access;
            barrier.SubresourceRange = Entry.Range;
            Entry.CurrentState = reqPhysState;
        }
    };

    for (auto& pass : OrderedPasses)
    {
        RHI::RHIDependencyDesc dependency;

        auto requireState = [&](RGTexture Texture, RGState ReqState)
        {
            RGTextureEntry& entry = *TextureResources[Texture.Index];
            transitionState(entry, ReqState, dependency);
        };

        for (auto& colorAttachment : pass->ColorAttachments)
        {
            requireState(colorAttachment.Texture, RGState::ColorAttachment);
        }

        if (pass->DepthAttachment.IsValid())
        {
            requireState(pass->DepthAttachment.Texture, RGState::DepthAttachment);
        }

        for (auto& access : pass->ReadAccesses)
        {
            requireState(access.Texture, access.State);
        }

        for (auto& access : pass->WriteAccesses)
        {
            requireState(access.Texture, access.State);
        }

        pass->CompiledDependency = std::move(dependency);

        BuildRenderingDesc(*pass);
    }

    for (auto& resource : TextureResources)
    {
        RGTextureEntry& entry = *resource;
        if (entry.IsImported && entry.FinalState != RGState::None)
        {
            transitionState(entry, entry.FinalState, CompiledExportDeps);
        }
    }
}

void RenderGraph::BuildRenderingDesc(RGPassBase& Pass)
{
    for (size_t i = 0; i < Pass.ColorAttachmentsDesc.size(); ++i)
    {
        const RGAttachment& attachment = Pass.ColorAttachments[i];
        RHI::RHIRenderingAttachmentDesc& desc = Pass.ColorAttachmentsDesc[i];
        desc.Attachment = TextureResources[attachment.Texture.Index]->View;
        desc.LoadOp = attachment.LoadOp;
        desc.StoreOp = attachment.StoreOp;
        desc.ClearValue.ClearColor = attachment.ClearValue.ClearColor;
    }

    if (Pass.DepthAttachment.IsValid())
    {
        const RGAttachment& attachment = Pass.DepthAttachment;
        RHI::RHIRenderingAttachmentDesc& desc = Pass.DepthAttachmentDesc;
        desc.Attachment = TextureResources[attachment.Texture.Index]->View;
        desc.LoadOp = attachment.LoadOp;
        desc.StoreOp = attachment.StoreOp;
        desc.ClearValue.DepthStencil.ClearDepth = attachment.ClearValue.DepthStencil.ClearDepth;
        desc.ClearValue.DepthStencil.ClearStencil = attachment.ClearValue.DepthStencil.ClearStencil;
    }

    RGTextureEntry& primary = *TextureResources[Pass.ColorAttachments[0].Texture.Index];
    Pass.RenderingDesc.RectangleExtent      = { primary.Image->GetWidth(),
                                               primary.Image->GetHeight() };
    Pass.RenderingDesc.LayerCount           = 1;
    Pass.RenderingDesc.ColorAttachments     = Pass.ColorAttachmentsDesc.data();
    Pass.RenderingDesc.ColorAttachmentCount = Pass.ColorAttachmentsDesc.size();
    Pass.RenderingDesc.DepthAttachment      = Pass.DepthAttachment.IsValid() ? &Pass.DepthAttachmentDesc : nullptr;
}
}
