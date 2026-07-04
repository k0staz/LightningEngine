#pragma once

#include "CoreMinimum.h"
#include <limits>
#include <utility>

#include "RHIDefinitions.h"
#include "ShaderCompiler.h"
#include "Containers/Array.h"
#include "Containers/ResourceArrays.h"
#include "Math/LinearColor.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

#undef max

namespace LE::RHI
{
/**
 * @brief Abstraction over API specific resources
 */
class RHIResource : public RefCountableBase
{
public:
    RHIResource(RHIResourceType InResourceType)
        : ResourceType(InResourceType)
    {
    }

    RHIResource(const RHIResource&) = delete;
    RHIResource& operator=(const RHIResource&) = delete;

    virtual ~RHIResource() = default;

    virtual bool IsValid() const = 0;

    inline RHIResourceType GetResourceType() const { return ResourceType; }

protected:
    RHIResourceType ResourceType;
};

class RHISampler : public RHIResource
{
public:
    RHISampler()
        : RHIResource(RHIResourceType::Sampler)
    {
    }

private:
    RHISamplerType Type = RHISamplerType::LinearRepeat;
};

struct RHIDescriptorSetLayoutBindingDesc
{
    uint32 Binding = 0;
    RHIDescriptorType DescriptorType = RHIDescriptorType::Sampler;
    uint32 DescriptorCount = 1;
    RHIShaderStage ShaderStage = RHIShaderStage::AllGraphics;
    std::vector<RefCountingPtr<RHISampler>> ImmutableSamplers;
};

struct RHIDescriptorSetLayoutDesc
{
    std::vector<RHIDescriptorBindingFlags> BindingFlags;
    std::vector<RHIDescriptorSetLayoutBindingDesc> Bindings;
    RHIDescriptorSetLayoutCreateFlags Flags = RHIDescriptorSetLayoutCreateFlags::None;
};

class RHIDescriptorSetLayout : public RHIResource
{
public:
    RHIDescriptorSetLayout(RHIDescriptorSetLayoutDesc InDesc)
        : RHIResource(RHIResourceType::DescriptorSetLayout)
          , Desc(InDesc)
    {
    }

    const RHIDescriptorSetLayoutDesc& GetDesc() const { return Desc; }

private:
    RHIDescriptorSetLayoutDesc Desc;
};

struct RHIDescriptorSetPoolDesc
{
    struct PoolSizes
    {
        RHIDescriptorType DescriptorType = RHIDescriptorType::Sampler;
        uint32 DescriptorCount = 0;
    };

    uint32 MaxSets = 0;
    RHIPoolCreateFlags Flags = RHIPoolCreateFlags::None;
    std::vector<PoolSizes> PoolSizes;
};

class RHIDescriptorSetPool : public RHIResource
{
public:
    RHIDescriptorSetPool(RHIDescriptorSetPoolDesc InDesc)
        : RHIResource(RHIResourceType::DescriptorSetPool)
          , Desc(InDesc)
    {
    }

    const RHIDescriptorSetPoolDesc& GetDesc() const { return Desc; }

private:
    RHIDescriptorSetPoolDesc Desc;
};

struct RHIDescriptorSetDesc
{
    RefCountingPtr<RHIDescriptorSetPool> Pool = nullptr;
    RefCountingPtr<RHIDescriptorSetLayout> Layout = nullptr;
};

class RHIDescriptorSet : public RHIResource
{
public:
    RHIDescriptorSet(RefCountingPtr<RHIDescriptorSetPool> InPool)
        : RHIResource(RHIResourceType::DescriptorSet)
          , Pool(InPool)
    {
    }

    RefCountingPtr<RHIDescriptorSetPool> GetPool() const { return Pool; }

private:
    RefCountingPtr<RHIDescriptorSetPool> Pool = nullptr;
};

struct RHIBufferDescription
{
    bool IsValid() const { return Size > 0 && UsageType != RHIBufferUsageType::Unknown; }

    uint64 Size = 0;
    RHIBufferUsageType UsageType = RHIBufferUsageType::Unknown;
};

/**
 * @brief Base abstraction for all graphics API buffer resources.
 *
 * Serves as the foundation for memory allocations. Extended into specialized
 * subclasses (RHIGlobalBuffer for GPU-only data, RHILinearBuffer for CPU-mapped data).
 */
class RHIBuffer : public RHIResource
{
public:
    RHIBuffer()
        : RHIResource(RHIResourceType::Buffer)
    {
    }

    RHIBuffer(RHIBufferDescription CreateDescription)
        : RHIResource(RHIResourceType::Buffer),
          Description(CreateDescription)
    {
    }

    RHIBufferUsageType GetUsageType() const { return Description.UsageType; }

    uint64 GetSize() const { return Description.Size; }

    template <typename T>
    T* GetAs()
    {
        return dynamic_cast<T*>(this);
    }

    template <typename T>
    const T* GetAs() const
    {
        return dynamic_cast<const T*>(this);
    }

    template <typename T>
    bool Is() const { return GetAs<T>() != nullptr; }

    bool IsGlobalBuffer() const;

protected:
    RHIBufferDescription Description;
};

/**
 * @brief Represents a sub-allocation within a global buffer.
 *
 * In order to write to this sub-allocation, the user needs to create a staging buffer.
 */
class RHIBufferSubAllocation : public RHIResource
{
public:
    RHIBufferSubAllocation()
        : RHIResource(RHIResourceType::BufferSubAllocation)
    {
    }

    RHIBufferSubAllocation(uint64 InSize, uint64 InOffset, uint64 InGpuAddress)
        : RHIResource(RHIResourceType::BufferSubAllocation),
          Size(InSize),
          Offset(InOffset),
          GpuAddress(InGpuAddress)
    {
    }

    bool IsSubAllocatedFrom(RHIBufferUsageType GlobalBufferUsageType) const;

    uint64 GetSize() const { return Size; }

    uint64 GetOffset() const { return Offset; }

    uint64 GetGpuAddress() const { return GpuAddress; }

protected:
    RHIBufferUsageType OwnerBufferUsageType = RHIBufferUsageType::Unknown;
    uint64 Size = 0;
    uint64 Offset = 0;
    uint64 GpuAddress = 0;
};

struct RHIGlobalBufferUploadDesc
{
    uint64 Size = 0;
    uint64 GlobalBufferOffset = 0;
    uint64 StageBufferOffset = 0;
};

class RHIGlobalBufferChannel : public RHIResource
{
public:
    RHIGlobalBufferChannel() :
        RHIResource(RHIResourceType::GlobalBufferChannel)
    {
    }

    RHIGlobalBufferChannel(RHIGlobalBufferChannelType InType, uint64 InSize, uint64 InOffset, uint64 InGpuAddress)
        : RHIResource(RHIResourceType::GlobalBufferChannel),
          Type(InType),
          Size(InSize),
          Offset(InOffset),
          GpuAddress(InGpuAddress)
    {
    }

    virtual RefCountingPtr<RHIBufferSubAllocation> CreateSubAllocation(uint64 InSize) = 0;
    virtual void FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation) = 0;

    uint64 GetSize() const { return Size; }

    uint64 GetOffset() const { return Offset; }

    uint64 GetGpuAddress() const { return GpuAddress; }

protected:
    RHIGlobalBufferChannelType Type = RHIGlobalBufferChannelType::Unknown;
    uint64 Size = 0;
    uint64 Offset = 0;
    uint64 GpuAddress = 0;
};

/**
 * @brief Represents a global buffer used for static, GPU-only resources.
 *
 * It is designed for sub-allocations. To write to this sub-allocated memory,
 * the user must use a staging buffer because global buffers are not mapped to CPU memory.
 *
 * @note This class supports a hierarchical allocation architecture. The buffer can either
 *       be sub-allocated directly, or partitioned into dedicated, type-safe data channels
 *       (e.g., Position, Indices) via RHIGlobalBufferChannel.
 */
class RHIGlobalBuffer : public RHIBuffer
{
public:
    RHIGlobalBuffer() = default;

    RHIGlobalBuffer(RHIBufferDescription CreateDescription)
        : RHIBuffer(CreateDescription)
    {
    }

    /**
    * @brief Allocates a raw memory slot directly out of the parent global buffer's root space.
    * @param Size The size of the requested sub-allocation in bytes.
    * @return A reference-counted pointer to the allocated memory node tracking context.
    * @note Enforces RHI-level memory alignment rules internally.
    */
    virtual RefCountingPtr<RHIBufferSubAllocation> CreateSubAllocation(uint64 Size) = 0;
    /**
     * @brief Frees a previously created root-level sub-allocation, recycling its memory.
     * @param SubAllocation The reference-counted pointer tracking the allocation node to release.
     */
    virtual void FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation) = 0;

    /**
     * @brief Fetches an existing dynamic data channel, or carves out a new one from the root block if it does not exist.
     * @param ChannelType The type-safe enum identifier for the requested channel (e.g., Positions, Normals).
     * @param Size The memory capacity in bytes to reserve for the channel if it needs to be initialized.
     * @return A reference-counted pointer to the initialized channel context ready for isolated sub-allocations.
     * @note Thread-safe.
     */
    virtual RefCountingPtr<RHIGlobalBufferChannel> GetCreateBufferChannel(RHIGlobalBufferChannelType ChannelType, uint64 Size) = 0;

    /**
    * @brief Destroys a specified data channel and releases its entire reserved block back to the parent root buffer.
    * @param ChannelType The type-safe enum identifier of the channel block to remove.
    * @warning Ensure all individual resource sub-allocations inside this channel have been freed before calling this.
    */
    virtual void RemoveBufferChannel(RHIGlobalBufferChannelType ChannelType) = 0;

protected:
    std::unordered_map<RHIGlobalBufferChannelType, RefCountingPtr<RHIGlobalBufferChannel>> ChannelMap;
};

/**
 * @brief An abstract wrapper representing a specific sub-allocation within a virtual memory block.
 *
 * This class tracks the mathematical range, size, and relative offset assigned by a virtual block allocator.
 * It serves as a backend-agnostic token used to reference unique slices of memory (like mesh data chunks or
 * transient staging buffer allocations) before they map to a physical hardware device address.
 */
class RHIVirtualMemoryAllocation : public RHIResource
{
public:
    RHIVirtualMemoryAllocation()
        : RHIResource(RHIResourceType::VirtualMemoryAllocation)
    {
    }

    virtual uint64 GetOffset() const = 0;
    virtual uint64 GetSize() const = 0;
};

/**
 * @brief An abstract wrapper around a purely software-driven mathematical memory block.
 *
 * This class isolates and exposes the bookkeeping functions of a virtual allocator (such as a VMA Virtual Block)
 * to manage an abstract address space. It handles arbitrary range packing, fragmentation tracking, and alignments
 * entirely on the CPU. It remains completely decoupled from physical GPU buffers until its calculated tracking
 * offsets are manually applied to hardware resources.
 */
class RHIVirtualMemoryBlock : public RHIResource
{
public:
    RHIVirtualMemoryBlock()
        : RHIResource(RHIResourceType::VirtualMemoryBlock)
    {
    }

    virtual RefCountingPtr<RHIVirtualMemoryAllocation> Allocate(uint64 InSize) = 0;
    virtual void FreeAllocation(RefCountingPtr<RHIVirtualMemoryAllocation> Allocation) = 0;
    virtual uint64 GetSize() const = 0;
    virtual uint64 GetFreeSize() const = 0;
};

/**
 * @brief Represents a linear buffer used for direct CPU-to-GPU writes.
 *
 * This memory is persistently mapped to the CPU, allowing immediate updates
 * (like per-frame data or staging transfers) without requiring GPU copy commands.
 */
class RHILinearBuffer : public RHIBuffer
{
public:
    RHILinearBuffer() = default;

    RHILinearBuffer(RHIBufferDescription CreateDescription)
        : RHIBuffer(CreateDescription)
    {
    }

    /**
     * @brief Writes raw CPU data directly into the persistently mapped buffer memory.
     *
     * @param RawData A pointer to the source data to be copied.
     * @param WriteSize The size of the data to copy, in bytes.
     * @return The precise 64-bit GPU virtual address corresponding to the written data region if
     *         the buffer supports device addresses; otherwise, returns 0.
     */
    virtual uint64 Write(const void* RawData, uint64 WriteSize) = 0;

    virtual void ResetBufferToDefault() = 0;
    virtual uint64 GetCurrentGpuAddress() const = 0;
    virtual uint64 GetUsedSize() const = 0;
};

struct RHIImageDesc
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 Depth = 0;
    uint32 MipLevels = 0;
    uint32 ArraySize = 0;

    RHIFormat Format = RHIFormat::None;
    RHIImageUsageFlag Usage = RHIImageUsageFlag::ColorAttachment;
};

class RHIImage : public RHIResource
{
public:
    RHIImage(const RHIImageDesc& CreateDescription)
        : RHIResource(RHIResourceType::Image),
          Desc(CreateDescription)
    {
    }

    uint32 GetWidth() const { return Desc.Width; }
    uint32 GetHeight() const { return Desc.Height; }
    uint32 GetDepth() const { return Desc.Depth; }
    uint32 GetMipLevels() const { return Desc.MipLevels; }
    uint32 GetArraySize() const { return Desc.ArraySize; }
    RHIFormat GetFormat() const { return Desc.Format; }
    RHIImageUsageFlag GetUsage() const { return Desc.Usage; }

protected:
    RHIImageDesc Desc;
};

struct RHISubresourceRange
{
    RHIImageAspectFlags Aspect = RHIImageAspectFlags::Color;
    uint32 BaseMipLevel = 0;
    uint32 NumMipLevels = 1;
    uint32 BaseArraySlice = 0;
    uint32 NumArraySlices = 1;
};

struct RHIImageViewDesc
{
    RefCountingPtr<RHIImage> Image = nullptr;
    RHIImageViewType ViewType = RHIImageViewType::None;
    RHIFormat Format = RHIFormat::None;
    RHISubresourceRange SubresourceRange = {};
};

class RHIImageView : public RHIResource
{
public:
    RHIImageView(const RHIImageViewDesc& DescIn)
        : RHIResource(RHIResourceType::ImageView),
          Desc(DescIn)
    {
    }

    RefCountingPtr<RHIImage> GetImage() const { return Desc.Image; }
    RHIImageViewType GetViewType() const { return Desc.ViewType; }
    RHIFormat GetFormat() const { return Desc.Format; }
    RHIImageAspectFlags GetAspect() const { return Desc.SubresourceRange.Aspect; }

    uint32 GetBaseMipLevel() const { return Desc.SubresourceRange.BaseMipLevel; }
    uint32 GetNumMipLevels() const { return Desc.SubresourceRange.NumMipLevels; }
    uint32 GetBaseArraySlice() const { return Desc.SubresourceRange.BaseArraySlice; }
    uint32 GetNumArraySlices() const { return Desc.SubresourceRange.NumArraySlices; }

protected:
    RHIImageViewDesc Desc;
};

struct RHIImageSubresourceLayers
{
    RHIImageAspectFlags Aspect = RHIImageAspectFlags::Color;
    uint32 MipLevel = 0;
    uint32 BaseArraySlice = 0;
    uint32 NumArraySlices = 0;
};

struct RHIBufferImageCopyDesc
{
    RefCountingPtr<RHIImage> Image = nullptr;

    struct CopyRegion
    {
        uint64 SourceBufferOffset = 0;
        Vector3U Extent = {0};
        Vector3I Offset = {0};
        RHIImageSubresourceLayers SubresourceLayers = {};
    };

    std::vector<CopyRegion> Regions;
};

struct RHIPipelineLayoutDesc
{
    RHIShaderStage ShaderStages = RHIShaderStage::None;
    uint32 PushConstantSize = 0;
    std::vector<RefCountingPtr<RHIDescriptorSetLayout>> DescriptorSetLayouts;

    bool operator==(const RHIPipelineLayoutDesc&) const = default;
};

class RHIPipelineLayout : public RHIResource
{
public:
    RHIPipelineLayout()
        : RHIResource(RHIResourceType::PipelineLayout)
    {
    }
};

struct RHIPipelineObjectDesc
{
    RefCountingPtr<RHIPipelineLayout> PipelineLayout;

    RefCountingPtr<ShaderModuleBlob> ShaderModule = nullptr;
    RHIShaderStage Stages = RHIShaderStage::None;
};

class RHIPipelineObject : public RHIResource
{
public:
    RHIPipelineObject()
        : RHIResource(RHIResourceType::PipelineObject)
    {
    }

    RHIPipelineObject(RefCountingPtr<RHIPipelineLayout> InPipelineLayout)
        : RHIResource(RHIResourceType::PipelineObject)
          , PipelineLayout(InPipelineLayout)
    {
    }

    RefCountingPtr<RHIPipelineLayout> GetPipelineLayout() const
    {
        return PipelineLayout;
    }

private:
    RefCountingPtr<RHIPipelineLayout> PipelineLayout = nullptr;
};

struct RHIWindowDesc
{
    void* NativeWindowHandle = nullptr;
    uint32 Width = 0;
    uint32 Height = 0;
};

class RHIWindow : public RHIResource
{
public:
    RHIWindow()
        : RHIResource(RHIResourceType::Window)
    {
    }

    RHIWindow(RefCountingPtr<RHIImage> InDepthImage, RefCountingPtr<RHIImageView> InDepthImageView,
              std::vector<RefCountingPtr<RHIImage>> InSwapchainImages, std::vector<RefCountingPtr<RHIImageView>> InSwapchainImageViews,
              RHIFormat InFormat, uint32 InWidth, uint32 InHeight)
        : RHIResource(RHIResourceType::Window),
          DepthImage(InDepthImage)
          , DepthImageView(InDepthImageView)
          , Width(InWidth)
          , Height(InHeight)
          , SwapchainImages(InSwapchainImages)
          , SwapchainImageViews(InSwapchainImageViews)
          , SwapChainFormat(InFormat)
    {
    }

    uint32 GetWidth() const { return Width; }

    uint32 GetHeight() const { return Height; }

    RefCountingPtr<RHIImage> GetDepthImage() const
    {
        return DepthImage;
    }

    RefCountingPtr<RHIImageView> GetDepthImageView() const
    {
        return DepthImageView;
    }

    RefCountingPtr<RHIImage> GetSwapchainImage(uint32 Index) const
    {
        return SwapchainImages[Index];
    }

    RefCountingPtr<RHIImageView> GetSwapchainImageView(uint32 Index) const
    {
        return SwapchainImageViews[Index];
    }

    uint32 GetSwapchainImageCount() const
    {
        return static_cast<uint32>(SwapchainImages.size());
    }

    RHIFormat GetSwapchainFormat() const
    {
        return SwapChainFormat;
    }

protected:
    RefCountingPtr<RHIImage> DepthImage;
    RefCountingPtr<RHIImageView> DepthImageView;

    std::vector<RefCountingPtr<RHIImage>> SwapchainImages;
    std::vector<RefCountingPtr<RHIImageView>> SwapchainImageViews;

    RHIFormat SwapChainFormat = RHIFormat::None;

    uint32 Width = 0;
    uint32 Height = 0;
};

struct RHIRenderingAttachmentDesc
{
    RefCountingPtr<RHIImageView> Attachment = nullptr;
    RHILoadOp LoadOp = RHILoadOp::Clear;
    RHIStoreOp StoreOp = RHIStoreOp::Store;

    union ClearValue
    {
        LinearColor ClearColor = {0.0f, 0.0f, 0.0f, 0.0f};

        struct
        {
            float ClearDepth = 0.0f;
            uint32 ClearStencil = 0;
        } DepthStencil;
    } ClearValue = {};
};

struct RHIRenderingDesc
{
    Vector2I RectangleOffset = Vector2I::Zero();
    Vector2U RectangleExtent = Vector2U::Zero();
    uint32 LayerCount = 1;
    uint32 ColorAttachmentCount = 0;
    const RHIRenderingAttachmentDesc* ColorAttachments = nullptr;
    const RHIRenderingAttachmentDesc* DepthAttachment = nullptr;
    const RHIRenderingAttachmentDesc* StencilAttachment = nullptr;
};

struct RHIImageMemoryBarrierDesc
{
    RefCountingPtr<RHIImage> Image = nullptr;
    RHIPipelineStageFlags SrcStageFlags = RHIPipelineStageFlags::None;
    RHIAccessFlags SrcAccessFlags = RHIAccessFlags::None;
    RHIPipelineStageFlags DstStageFlags = RHIPipelineStageFlags::None;
    RHIAccessFlags DstAccessFlags = RHIAccessFlags::None;
    RHIImageLayout OldLayout = RHIImageLayout::None;
    RHIImageLayout NewLayout = RHIImageLayout::None;
    std::optional<uint32> SrcQueueFamilyIndex;
    std::optional<uint32> DstQueueFamilyIndex;

    RHISubresourceRange SubresourceRange = {};
};

struct RHIDependencyDesc
{
    std::vector<RHIImageMemoryBarrierDesc> ImageMemoryBarriers;
};

struct RHIViewportDesc
{
    Vector2F Coordinates = {0};
    float Width = 0;
    float Height = 0;
    float MinDepth = 0;
    float MaxDepth = 0;
};

struct RHIPushConstantsDesc
{
    RefCountingPtr<RHIPipelineLayout> PipelineLayout = nullptr;
    RHIShaderStage ShaderStage = RHIShaderStage::None;
    uint32 Offset = 0;
    uint32 Size = 0;
    const void* Data = nullptr;
};

class RHICommandList : public RHIResource
{
public:
    RHICommandList()
        : RHIResource(RHIResourceType::CommandList)
    {
    }

    RHICommandList(RHICommandListType InType)
        : RHIResource(RHIResourceType::CommandList),
          ListType(InType)
    {
    }

    virtual void BeginRecording() = 0;
    virtual void EndRecording() = 0;

    virtual void PipelineBarrier(const RHIDependencyDesc& DependencyDesc) = 0;

    virtual void BeginRendering(const RHIRenderingDesc& RenderingDesc) = 0;
    virtual void EndRendering() = 0;

    virtual void SetViewport(const RHIViewportDesc& ViewportDesc) = 0;
    virtual void SetScissor(Vector2U Extent, Vector2I Offset = {0}) = 0;

    virtual void BindPipeline(RefCountingPtr<RHIPipelineObject> PipelineObject) = 0;

    virtual void PushConstants(const RHIPushConstantsDesc& PushConstantsDesc) = 0;

    virtual void Draw(uint32 IndexCount, uint32 InstanceCount = 1, uint32 FirstIndex = 0, int32 VertexOffset = 0,
                      uint32 FirstInstance = 0) = 0;

    virtual void CopyToGlobalBuffer(RefCountingPtr<RHIGlobalBuffer> GlobalBuffer, RefCountingPtr<RHILinearBuffer> StageBuffer,
                                    const std::vector<RHIGlobalBufferUploadDesc>& Descriptions) = 0;

    virtual void CopyBufferToImage(RefCountingPtr<RHILinearBuffer> StageBuffer, const RHIBufferImageCopyDesc& Desc) = 0;

    virtual void BindDescriptorSets(RefCountingPtr<RHIPipelineLayout> PipelineLayout, const std::vector<RefCountingPtr<RHI::RHIDescriptorSet>>& DescriptorSets) = 0;

    RHICommandListType GetListType() const { return ListType; }

private:
    RHICommandListType ListType = RHICommandListType::Unknown;
};

struct RHIDescriptorImageInfoDesc
{
    RefCountingPtr<RHIImageView> View = nullptr;
    RefCountingPtr<RHISampler> Sampler = nullptr;
    RHIImageLayout Layout = RHIImageLayout::None;
};

struct RHIUpdateDescriptorSetDesc
{
    std::vector<RHIDescriptorImageInfoDesc> ImageInfos;
    RefCountingPtr<RHIDescriptorSet> Set = nullptr;
    uint32 Binding = 0;
    uint32 ArrayElement = 0;
    RHIDescriptorType DescriptorType = RHIDescriptorType::Sampler;
};
}
