#include "VulkanResources.h"

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

#include "VulkanUtils.h"

namespace LE::RHI::Vulkan
{
void VulkanBuffer::Reset()
{
    Handle = nullptr;
    Allocation = nullptr;
    DeviceAddress = 0;
}

bool VulkanBuffer::IsBaseValid() const
{
    return Handle != nullptr && Allocation != nullptr;
}

bool VulkanSubAllocation::IsValid() const
{
    return VirtualAllocation != nullptr;
}

void VulkanSubAllocation::Reset()
{
    VirtualAllocation = nullptr;
    Size = 0;
    Offset = 0;
    GpuAddress = 0;
    OwnerBufferUsageType = RHIBufferUsageType::Unknown;
}

bool VulkanGlobalBufferChannel::IsValid() const
{
    return Type != RHIGlobalBufferChannelType::Unknown && VirtualAllocation && VirtualMemoryBlock;
}

void VulkanGlobalBufferChannel::Reset()
{
    VirtualAllocation = nullptr;
    Size = 0;
    Offset = 0;
    GpuAddress = 0;
    Type = RHIGlobalBufferChannelType::Unknown;
    vmaDestroyVirtualBlock(VirtualMemoryBlock);
}

RefCountingPtr<RHIBufferSubAllocation> VulkanGlobalBufferChannel::CreateSubAllocation(uint64 InSize)
{
    VmaVirtualAllocationCreateInfo subAllocInfo = {};
    subAllocInfo.size = InSize;
    subAllocInfo.alignment = RHI::GlobalStorageAlignment;

    VmaVirtualAllocation allocation = nullptr;
    uint64 offset = 0;
    VkResult result = VK_ERROR_UNKNOWN;
    {
        std::unique_lock lock(VirtualBlockMutex);
        result = vmaVirtualAllocate(VirtualMemoryBlock, &subAllocInfo, &allocation, &offset);
    }
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }
    uint64 globalBufferOffset = Offset + offset;

    return new VulkanSubAllocation(InSize, globalBufferOffset, GpuAddress + offset, allocation);
}

void VulkanGlobalBufferChannel::FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation)
{
    if (!SubAllocation || !SubAllocation->IsValid())
    {
        return;
    }

    VulkanSubAllocation* subAllocation = ResourceCast(SubAllocation.GetPointer());
    {
        std::unique_lock lock(VirtualBlockMutex);
        vmaVirtualFree(VirtualMemoryBlock, subAllocation->GetVirtualAllocation());
    }

    subAllocation->Reset();
}

VulkanGlobalBuffer::~VulkanGlobalBuffer()
{
    LE_ASSERT_DESC(!IsValid(), "Leaked Buffer Resource")
}

bool VulkanGlobalBuffer::IsValid() const
{
    return VulkanBuffer::IsBaseValid() && DeviceAddress > 0;
}

void VulkanGlobalBuffer::Reset()
{
    VulkanBuffer::Reset();
    vmaDestroyVirtualBlock(VirtualMemoryBlock);
}

RefCountingPtr<RHIBufferSubAllocation> VulkanGlobalBuffer::CreateSubAllocation(uint64 Size)
{
    VmaVirtualAllocationCreateInfo subAllocInfo = {};
    subAllocInfo.size = Size;
    subAllocInfo.alignment = RHI::GlobalStorageAlignment;

    VmaVirtualAllocation allocation = nullptr;
    uint64 offset = 0;
    VkResult result = VK_ERROR_UNKNOWN;
    {
        std::unique_lock lock(VirtualBlockMutex);
        result = vmaVirtualAllocate(VirtualMemoryBlock, &subAllocInfo, &allocation, &offset);
    }
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    return new VulkanSubAllocation(Size, offset, DeviceAddress + offset, allocation);
}

void VulkanGlobalBuffer::FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation)
{
    if (!SubAllocation || !SubAllocation->IsValid() || !SubAllocation->IsSubAllocatedFrom(Description.UsageType))
    {
        return;
    }

    VulkanSubAllocation* subAllocation = ResourceCast(SubAllocation.GetPointer());
    {
        std::unique_lock lock(VirtualBlockMutex);
        vmaVirtualFree(VirtualMemoryBlock, subAllocation->GetVirtualAllocation());
    }

    subAllocation->Reset();
}

RefCountingPtr<RHIGlobalBufferChannel> VulkanGlobalBuffer::GetCreateBufferChannel(RHIGlobalBufferChannelType ChannelType, uint64 Size)
{
    if (ChannelType == RHIGlobalBufferChannelType::Unknown || ChannelMap.contains(ChannelType))
    {
        LE_ASSERT_DESC(false, "Invalid Channel Type request")
        return nullptr;
    }

    VmaVirtualAllocationCreateInfo subAllocInfo = {};
    subAllocInfo.size = Size;
    subAllocInfo.alignment = RHI::GlobalStorageAlignment;

    VmaVirtualAllocation allocation = nullptr;
    uint64 offset = 0;
    VkResult result = VK_ERROR_UNKNOWN;
    {
        std::unique_lock lock(VirtualBlockMutex);
        result = vmaVirtualAllocate(VirtualMemoryBlock, &subAllocInfo, &allocation, &offset);
    }
    if (result != VK_SUCCESS)
    {
        return nullptr;
    }

    VmaVirtualBlockCreateInfo blockCI = {};
    blockCI.size = Size;

    VmaVirtualBlock block;
    CheckVkResult(vmaCreateVirtualBlock(&blockCI, &block));

    VulkanGlobalBufferChannel* channel = new
        VulkanGlobalBufferChannel(ChannelType, Size, offset, DeviceAddress + offset, allocation, block);
    ChannelMap.insert(std::make_pair(ChannelType, channel));
    return channel;
}

void VulkanGlobalBuffer::RemoveBufferChannel(RHIGlobalBufferChannelType ChannelType)
{
    auto it = ChannelMap.find(ChannelType);
    if (it == ChannelMap.end())
    {
        return;
    }

    VulkanGlobalBufferChannel* channel = ResourceCast(it->second.GetPointer());
    if (!channel || !channel->IsValid())
    {
        return;
    }

    {
        std::unique_lock lock(VirtualBlockMutex);
        vmaVirtualFree(VirtualMemoryBlock, channel->GetVirtualAllocation());
    }

    ChannelMap.erase(ChannelType);
    channel->Reset();
}

VulkanLinearBuffer::~VulkanLinearBuffer()
{
    LE_ASSERT_DESC(!IsValid(), "Leaked Buffer Resource")
}

bool VulkanLinearBuffer::IsValid() const
{
    bool result = VulkanBuffer::IsBaseValid();
    switch (GetUsageType())
    {
    case RHIBufferUsageType::DynamicFrameData:
        result = result && DeviceAddress > 0 && MappedMemory != nullptr;
        break;
    case RHIBufferUsageType::UploadStaging:
        result = result && MappedMemory != nullptr;
        break;
    default:
        LE_ASSERT_DESC(false, "Invalid Buffer Usage")
        result = false;
        break;
    }
    return result;
}

void VulkanLinearBuffer::Reset()
{
    VulkanBuffer::Reset();
    MappedMemory = nullptr;
}

uint64 VulkanLinearBuffer::Write(const void* RawData, uint64 WriteSize)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Attempting to write to an invalid buffer")
        return CurrentDeviceAddress;
    }

    if (WriteSize > Description.Size - UsedSize)
    {
        LE_ASSERT_DESC(false, "Attempting to write more than the buffer has");
        return CurrentDeviceAddress;
    }

    std::memcpy(CurrentMappedMemory, RawData, WriteSize);

    UsedSize += WriteSize;
    CurrentMappedMemory = static_cast<char*>(MappedMemory) + UsedSize;
    if (CurrentDeviceAddress != 0)
    {
        CurrentDeviceAddress += WriteSize;
    }

    return CurrentDeviceAddress;
}

void VulkanLinearBuffer::ResetBufferToDefault()
{
    CurrentDeviceAddress = DeviceAddress;
    CurrentMappedMemory = MappedMemory;
    UsedSize = 0;
}

uint64 VulkanLinearBuffer::GetCurrentGpuAddress() const
{
    return CurrentDeviceAddress;
}

uint64 VulkanLinearBuffer::GetUsedSize() const
{
    return UsedSize;
}

VmaVirtualMemoryAllocation::~VmaVirtualMemoryAllocation()
{
    LE_ASSERT_DESC(VirtualAllocation == nullptr, "Leaked Virtual Memory Allocation")
}

void VmaVirtualMemoryAllocation::Reset()
{
    VirtualAllocation = nullptr;
    Offset = 0;
    Size = 0;
}

bool VmaVirtualMemoryAllocation::IsValid() const
{
    return VirtualAllocation != nullptr;
}

uint64 VmaVirtualMemoryAllocation::GetOffset() const
{
    return Offset;
}

uint64 VmaVirtualMemoryAllocation::GetSize() const
{
    return Size;
}

VmaVirtualAllocation VmaVirtualMemoryAllocation::GetVirtualAllocation() const
{
    return VirtualAllocation;
}

bool VmaVirtualMemoryBlock::IsValid() const
{
    return VirtualMemoryBlock != nullptr;
}

RefCountingPtr<RHIVirtualMemoryAllocation> VmaVirtualMemoryBlock::Allocate(uint64 InSize)
{
    VmaVirtualAllocationCreateInfo subAllocInfo = {};
    subAllocInfo.size = Size;
    subAllocInfo.alignment = RHI::GlobalStorageAlignment;

    VmaVirtualAllocation allocation = nullptr;
    uint64 offset = 0;
    {
        std::unique_lock lock(VirtualBlockMutex);
        VkResult result = vmaVirtualAllocate(VirtualMemoryBlock, &subAllocInfo, &allocation, &offset);
        if (result != VK_SUCCESS)
        {
            return nullptr;
        }

        AllocatedSize += subAllocInfo.size;
    }

    return new VmaVirtualMemoryAllocation(allocation, subAllocInfo.size, offset);
}

void VmaVirtualMemoryBlock::FreeAllocation(RefCountingPtr<RHIVirtualMemoryAllocation> Allocation)
{
    if (!Allocation || !Allocation->IsValid())
    {
        return;
    }

    VmaVirtualMemoryAllocation* subAllocation = ResourceCast(Allocation.GetPointer());
    {
        std::unique_lock lock(VirtualBlockMutex);

        vmaVirtualFree(VirtualMemoryBlock, subAllocation->GetVirtualAllocation());

        AllocatedSize -= subAllocation->GetSize();
    }

    subAllocation->Reset();
}

uint64 VmaVirtualMemoryBlock::GetSize() const
{
    if (!IsValid())
    {
        return 0;
    }

    return Size;
}

uint64 VmaVirtualMemoryBlock::GetFreeSize() const
{
    std::unique_lock lock(VirtualBlockMutex);
    return Size - AllocatedSize;
}

bool VulkanCommandList::IsValid() const
{
    return VulkanCommandBuffer != nullptr;
}

void VulkanCommandList::BeginRecording()
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Attempting to begin recording a non-valid command buffer")
        return;
    }

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };

    CheckVkResult(vkBeginCommandBuffer(VulkanCommandBuffer, &beginInfo));
}

void VulkanCommandList::EndRecording()
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Attempting to end recording a non-valid command buffer")
        return;
    }

    CheckVkResult(vkEndCommandBuffer(VulkanCommandBuffer));
}

void VulkanCommandList::PipelineBarrier(const RHIDependencyDesc& DependencyDesc)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Attempting to put pipeline barrier on a non-valid command buffer")
        return;
    }

    std::vector<VkImageMemoryBarrier2> imageBarriers(DependencyDesc.ImageMemoryBarriers.size());
    for (size_t i = 0; i < DependencyDesc.ImageMemoryBarriers.size(); ++i)
    {
        const RHIImageMemoryBarrierDesc& imageBarrier = DependencyDesc.ImageMemoryBarriers[i];
        VkImageMemoryBarrier2& vkBarrier = imageBarriers[i];
        vkBarrier = {};
        vkBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        vkBarrier.srcStageMask = MapPipelineStages(imageBarrier.SrcStageFlags);
        vkBarrier.srcAccessMask = MapAccessFlags(imageBarrier.SrcAccessFlags);
        vkBarrier.dstStageMask = MapPipelineStages(imageBarrier.DstStageFlags);
        vkBarrier.dstAccessMask = MapAccessFlags(imageBarrier.DstAccessFlags);
        vkBarrier.oldLayout = MapImageLayout(imageBarrier.OldLayout);
        vkBarrier.newLayout = MapImageLayout(imageBarrier.NewLayout);
        vkBarrier.image = ResourceCast(imageBarrier.Image.GetPointer())->GetVkImage();
        vkBarrier.subresourceRange = MapSubresourceRange(imageBarrier.SubresourceRange);
        if (imageBarrier.SrcQueueFamilyIndex.has_value())
        {
            vkBarrier.srcQueueFamilyIndex = imageBarrier.SrcQueueFamilyIndex.value();
        }
        if (imageBarrier.DstQueueFamilyIndex.has_value())
        {
            vkBarrier.dstQueueFamilyIndex = imageBarrier.DstQueueFamilyIndex.value();
        }
    }

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = imageBarriers.size();
    dependencyInfo.pImageMemoryBarriers = imageBarriers.data();

    vkCmdPipelineBarrier2(VulkanCommandBuffer, &dependencyInfo);
}

void VulkanCommandList::BeginRendering(const RHIRenderingDesc& RenderingDesc)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to start rendering using a non-valid command buffer");
        return;
    }

    std::vector<VkRenderingAttachmentInfo> vkColorAttachments(RenderingDesc.ColorAttachmentCount);
    for (uint32 i = 0; i < RenderingDesc.ColorAttachmentCount; ++i)
    {
        vkColorAttachments[i] = MapRenderingAttachmentInfo(RenderingDesc.ColorAttachments[i]);
    }

    const bool hasDepth = RenderingDesc.DepthAttachment != nullptr;
    VkRenderingAttachmentInfo depthAttachmentInfo = {};
    if (hasDepth)
    {
        depthAttachmentInfo = MapRenderingAttachmentInfo(*RenderingDesc.DepthAttachment);
    }

    const bool hasStencil = RenderingDesc.StencilAttachment != nullptr;
    VkRenderingAttachmentInfo stencilAttachmentInfo = {};
    if (hasStencil)
    {
        stencilAttachmentInfo = MapRenderingAttachmentInfo(*RenderingDesc.StencilAttachment);
    }

    VkRenderingInfo vkRenderingInfo = {};
    vkRenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    vkRenderingInfo.renderArea = {
        .offset = {.x = RenderingDesc.RectangleOffset.X, .y = RenderingDesc.RectangleOffset.Y},
        .extent = {.width = RenderingDesc.RectangleExtent.X, .height = RenderingDesc.RectangleExtent.Y}
    };
    vkRenderingInfo.layerCount = RenderingDesc.LayerCount;
    vkRenderingInfo.colorAttachmentCount = RenderingDesc.ColorAttachmentCount;
    vkRenderingInfo.pColorAttachments = vkColorAttachments.data();
    vkRenderingInfo.pDepthAttachment = hasDepth ? &depthAttachmentInfo : nullptr;
    vkRenderingInfo.pStencilAttachment = hasStencil ? &stencilAttachmentInfo : nullptr;

    vkCmdBeginRendering(VulkanCommandBuffer, &vkRenderingInfo);
}

void VulkanCommandList::EndRendering()
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    vkCmdEndRendering(VulkanCommandBuffer);
}

void VulkanCommandList::SetViewport(const RHIViewportDesc& ViewportDesc)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    VkViewport vkViewport = {};
    vkViewport.x = ViewportDesc.Coordinates.X;
    vkViewport.y = ViewportDesc.Coordinates.Y;
    vkViewport.width = ViewportDesc.Width;
    vkViewport.height = -ViewportDesc.Height;
    vkViewport.minDepth = ViewportDesc.MinDepth;
    vkViewport.maxDepth = ViewportDesc.MaxDepth;
    vkCmdSetViewport(VulkanCommandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandList::SetScissor(Vector2U Extent, Vector2I Offset)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    VkRect2D vkRect = {};
    vkRect.offset.x = Offset.X;
    vkRect.offset.y = Offset.Y;
    vkRect.extent.width = Extent.X;
    vkRect.extent.height = Extent.Y;
    vkCmdSetScissor(VulkanCommandBuffer, 0, 1, &vkRect);
}

void VulkanCommandList::BindPipeline(RefCountingPtr<RHIPipelineObject> PipelineObject)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    if (!PipelineObject || !PipelineObject->IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid pipeline object")
        return;
    }

    VulkanPipelineObject* pipelineObject = ResourceCast(PipelineObject.GetPointer());
    vkCmdBindPipeline(VulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineObject->GetVkPipeline());
}

void VulkanCommandList::PushConstants(const RHIPushConstantsDesc& PushConstantsDesc)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    if (!PushConstantsDesc.PipelineLayout || !PushConstantsDesc.Data)
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid push constants data")
        return;
    }

    VulkanPipelineLayout* pipelineLayout = ResourceCast(PushConstantsDesc.PipelineLayout.GetPointer());
    vkCmdPushConstants(VulkanCommandBuffer, pipelineLayout->GetVkPipelineLayout(), ShaderStageFlagBit(PushConstantsDesc.ShaderStage),
                       PushConstantsDesc.Offset, PushConstantsDesc.Size, PushConstantsDesc.Data);
}

void VulkanCommandList::Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    vkCmdDraw(VulkanCommandBuffer, IndexCount, InstanceCount, FirstIndex, FirstInstance);
}

void VulkanCommandList::CopyToGlobalBuffer(RefCountingPtr<RHIGlobalBuffer> GlobalBuffer, RefCountingPtr<RHILinearBuffer> StageBuffer,
                                           const std::vector<RHIGlobalBufferUploadDesc>& Descriptions)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    if (Descriptions.empty())
    {
        return;
    }

    if (!StageBuffer.IsValid() || !StageBuffer->IsValid())
    {
        LE_ASSERT_DESC(false, "Invalid stage buffer")
        return;
    }

    if (!GlobalBuffer || !GlobalBuffer->IsValid())
    {
        LE_ASSERT_DESC(false, "Invalid global buffer")
        return;
    }

    std::vector<VkBufferCopy2> copyRegions;
    copyRegions.reserve(Descriptions.size());
    for (const auto& desc : Descriptions)
    {
        VkBufferCopy2 region = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
            .srcOffset = desc.StageBufferOffset,
            .dstOffset = desc.GlobalBufferOffset,
            .size = desc.Size
        };
        copyRegions.emplace_back(region);
    }

    VkCopyBufferInfo2 copyInfo = {
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
        .pNext = nullptr,
        .srcBuffer = ResourceCast(StageBuffer.GetPointer())->GetHandle(),
        .dstBuffer = ResourceCast(GlobalBuffer.GetPointer())->GetHandle(),
        .regionCount = static_cast<uint32_t>(copyRegions.size()),
        .pRegions = copyRegions.data()
    };

    vkCmdCopyBuffer2(VulkanCommandBuffer, &copyInfo);
}

void VulkanCommandList::CopyBufferToImage(RefCountingPtr<RHILinearBuffer> StageBuffer, const RHIBufferImageCopyDesc& Desc)
{
    if (!IsValid() || !StageBuffer || !StageBuffer->IsValid())
    {
        return;
    }

    if (!Desc.Image || !Desc.Image->IsValid())
    {
        return;
    }

    VulkanLinearBuffer* stageBuffer = ResourceCast(StageBuffer.GetPointer());
    VulkanImage* image = ResourceCast(Desc.Image.GetPointer());

    std::vector<VkBufferImageCopy> copyRegions;
    copyRegions.reserve(Desc.Regions.size());
    for (const auto& region : Desc.Regions)
    {
        VkBufferImageCopy& copyRegion = copyRegions.emplace_back();
        copyRegion.bufferOffset = region.SourceBufferOffset;
        copyRegion.imageExtent = {region.Extent.X, region.Extent.Y, region.Extent.Z};
        copyRegion.imageOffset = {region.Offset.X, region.Offset.Y, region.Offset.Z};
        copyRegion.imageSubresource = MapSubresourceLayers(region.SubresourceLayers);
    }

    vkCmdCopyBufferToImage(VulkanCommandBuffer, stageBuffer->GetHandle(), image->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           copyRegions.size(), copyRegions.data());
}

void VulkanCommandList::BindDescriptorSets(RefCountingPtr<RHIPipelineLayout> PipelineLayout,
                                           const std::vector<RefCountingPtr<RHI::RHIDescriptorSet>>& DescriptorSets)
{
    if (!IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to use a non-valid command buffer")
        return;
    }

    if (!PipelineLayout || !PipelineLayout->IsValid())
    {
        LE_ASSERT_DESC(false, "Trying to bind invalid pipeline layout");
        return;
    }

    if (DescriptorSets.empty())
    {
        LE_WARN("Trying to bind empty descriptor sets");
        return;
    }

    std::vector<VkDescriptorSet> descriptorSets;
    descriptorSets.reserve(DescriptorSets.size());
    for (const auto& descriptorSet : DescriptorSets)
    {
        if (!descriptorSet || !descriptorSet->IsValid())
        {
            LE_ERROR("Trying to bind invalid descriptor set");
            continue;
        }

        descriptorSets.push_back(ResourceCast(descriptorSet.GetPointer())->GetVkDescriptorSet());
    }

    vkCmdBindDescriptorSets(VulkanCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            ResourceCast(PipelineLayout.GetPointer())->GetVkPipelineLayout(), 0, descriptorSets.size(),
                            descriptorSets.data(), 0, nullptr);
}
}
