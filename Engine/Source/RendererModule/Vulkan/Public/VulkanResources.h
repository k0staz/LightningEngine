#pragma once
#include <mutex>

#include "RHIResources.h"
#include "VulkanFwd.h"

namespace LE::RHI::Vulkan
{
class VulkanBuffer
{
public:
    virtual ~VulkanBuffer() = default;

    VulkanBuffer(VkBuffer InHandle, VmaAllocation InAllocation, VkDeviceAddress InAddress)
        :
        Handle(InHandle),
        Allocation(InAllocation),
        DeviceAddress(InAddress)
    {
    }

    VkBuffer GetHandle() const { return Handle; }

    VmaAllocation GetAllocation() const { return Allocation; }

    virtual void Reset();

protected:
    bool IsBaseValid() const;

protected:
    VkBuffer Handle = nullptr;
    VmaAllocation Allocation = nullptr;
    VkDeviceAddress DeviceAddress = 0;
};

class VulkanSubAllocation final : public RHIBufferSubAllocation
{
public:
    VulkanSubAllocation(uint64 InSize, uint64 InOffset, uint64 IsGpuAddress, VmaVirtualAllocation InVirtualAllocation)
        : RHIBufferSubAllocation(InSize, InOffset, IsGpuAddress),
          VirtualAllocation(InVirtualAllocation)
    {
    }

    bool IsValid() const override;
    void Reset();

    VmaVirtualAllocation GetVirtualAllocation() const { return VirtualAllocation; }

private:
    VmaVirtualAllocation VirtualAllocation = nullptr;
};

class VulkanGlobalBufferChannel final : public RHIGlobalBufferChannel
{
public:
    VulkanGlobalBufferChannel(RHIGlobalBufferChannelType InType, uint64 InSize, uint64 InOffset, uint64 IsGpuAddress,
                              VmaVirtualAllocation InVirtualAllocation, VmaVirtualBlock InVirtualMemoryBlock)
        : RHIGlobalBufferChannel(InType, InSize, InOffset, IsGpuAddress),
          VirtualAllocation(InVirtualAllocation),
          VirtualMemoryBlock(InVirtualMemoryBlock)
    {
    }

    bool IsValid() const override;
    void Reset();

    VmaVirtualAllocation GetVirtualAllocation() const { return VirtualAllocation; }

    RefCountingPtr<RHIBufferSubAllocation> CreateSubAllocation(uint64 InSize) override;
    void FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation) override;

private:
    VmaVirtualAllocation VirtualAllocation = nullptr;
    VmaVirtualBlock VirtualMemoryBlock = nullptr;
    std::mutex VirtualBlockMutex;
};

class VulkanGlobalBuffer final : public RHIGlobalBuffer, public VulkanBuffer
{
public:
    VulkanGlobalBuffer(VkBuffer InHandle, VmaAllocation InAllocation, VkDeviceAddress InAddress, VmaVirtualBlock VirtualMemoryBlock,
                       RHIBufferDescription InDescription)
        : RHIGlobalBuffer(InDescription),
          VulkanBuffer(InHandle, InAllocation, InAddress),
          VirtualMemoryBlock(VirtualMemoryBlock)
    {
    }

    ~VulkanGlobalBuffer() override;

    bool IsValid() const override;
    void Reset() override;

    RefCountingPtr<RHIBufferSubAllocation> CreateSubAllocation(uint64 Size) override;
    void FreeSubAllocation(RefCountingPtr<RHIBufferSubAllocation> SubAllocation) override;

    RefCountingPtr<RHIGlobalBufferChannel> GetCreateBufferChannel(RHIGlobalBufferChannelType ChannelType, uint64 Size) override;
    void RemoveBufferChannel(RHIGlobalBufferChannelType ChannelType) override;

private:
    VmaVirtualBlock VirtualMemoryBlock;
    std::mutex VirtualBlockMutex;
};

class VulkanLinearBuffer final : public RHILinearBuffer, public VulkanBuffer
{
public:
    VulkanLinearBuffer(VkBuffer InHandle, VmaAllocation InAllocation, VkDeviceAddress InAddress, void* InMappedMemmory,
                       RHIBufferDescription InDescription)
        : RHILinearBuffer(InDescription),
          VulkanBuffer(InHandle, InAllocation, InAddress),
          MappedMemory(InMappedMemmory),
          CurrentMappedMemory(InMappedMemmory),
          CurrentDeviceAddress(InAddress)
    {
    }

    ~VulkanLinearBuffer() override;

    bool IsValid() const override;
    void Reset() override;
    uint64 Write(const void* RawData, uint64 WriteSize) override;
    void ResetBufferToDefault() override;
    uint64 GetCurrentGpuAddress() const override;
    uint64 GetUsedSize() const override;

private:
    void* MappedMemory = nullptr;
    void* CurrentMappedMemory = nullptr;
    uint64 UsedSize = 0;
    VkDeviceAddress CurrentDeviceAddress = 0;
};

class VmaVirtualMemoryAllocation final : public RHIVirtualMemoryAllocation
{
public:
    VmaVirtualMemoryAllocation(VmaVirtualAllocation InVmaVirtualAllocation, uint64 InSize, uint64 InOffset)
        : RHIVirtualMemoryAllocation(),
          VirtualAllocation(InVmaVirtualAllocation),
          Size(InSize),
          Offset(InOffset)
    {
    }

    ~VmaVirtualMemoryAllocation() override;

    void Reset();
    bool IsValid() const override;
    uint64 GetOffset() const override;
    uint64 GetSize() const override;

    VmaVirtualAllocation GetVirtualAllocation() const;

private:
    VmaVirtualAllocation VirtualAllocation = nullptr;
    uint64 Size = 0;
    uint64 Offset = 0;
};

class VmaVirtualMemoryBlock final : public RHIVirtualMemoryBlock
{
public:
    VmaVirtualMemoryBlock(VmaVirtualBlock InVirtualMemoryBlock, uint64 InSize)
        : RHIVirtualMemoryBlock(),
          Size(InSize),
          AllocatedSize(0),
          VirtualMemoryBlock(InVirtualMemoryBlock)
    {
    }

    bool IsValid() const override;
    RefCountingPtr<RHIVirtualMemoryAllocation> Allocate(uint64 InSize) override;
    void FreeAllocation(RefCountingPtr<RHIVirtualMemoryAllocation> Allocation) override;
    uint64 GetSize() const override;
    uint64 GetFreeSize() const override;

private:
    uint64 Size;
    uint64 AllocatedSize;
    VmaVirtualBlock VirtualMemoryBlock;
    mutable std::mutex VirtualBlockMutex;
};

class VulkanCommandList final : public RHICommandList
{
public:
    VulkanCommandList(VkCommandBuffer InCommandBuffer, RHICommandListType InType)
        : RHICommandList(InType)
          , VulkanCommandBuffer(InCommandBuffer)
    {
    }

    bool IsValid() const override;

    VkCommandBuffer GetVkCommandBuffer() const { return VulkanCommandBuffer; }

    void BeginRecording() override;
    void EndRecording() override;
    void PipelineBarrier(const RHIDependencyDesc& DependencyDesc) override;
    void BeginRendering(const RHIRenderingDesc& RenderingDesc) override;
    void EndRendering() override;
    void SetViewport(const RHIViewportDesc& ViewportDesc) override;
    void SetScissor(Vector2U Extent, Vector2I Offset) override;
    void BindPipeline(RefCountingPtr<RHIPipelineObject> PipelineObject) override;
    void PushConstants(const RHIPushConstantsDesc& PushConstantsDesc) override;
    void Draw(uint32 IndexCount, uint32 InstanceCount, uint32 FirstIndex, int32 VertexOffset, uint32 FirstInstance) override;

private:
    VkCommandBuffer VulkanCommandBuffer = nullptr;
};

class VulkanPipelineLayout final : public RHIPipelineLayout
{
public:
    VulkanPipelineLayout(VkPipelineLayout InLayout)
        : RHIPipelineLayout()
          , Layout(InLayout)
    {
    }

    VkPipelineLayout GetVkPipelineLayout() const { return Layout; }
    bool IsValid() const override { return Layout != nullptr; }

private:
    VkPipelineLayout Layout = nullptr;
};

class VulkanPipelineObject final : public RHIPipelineObject
{
public:
    VulkanPipelineObject(VkPipeline Pipeline, RefCountingPtr<RHIPipelineLayout> InPipelineLayout)
        : RHIPipelineObject(InPipelineLayout)
          , Pipeline(Pipeline)
    {
    }

    VkPipeline GetVkPipeline() const { return Pipeline; }

    bool IsValid() const override { return Pipeline != nullptr; }

private:
    VkPipeline Pipeline = nullptr;
};

class VulkanImage final : public RHIImage
{
public:
    VulkanImage(const RHIImageDesc& CreateDescription, VkImage Image, VmaAllocation InAllocation)
        : RHIImage(CreateDescription),
          Image(Image),
          Allocation(InAllocation)
    {
    }

    bool IsValid() const override { return Image != nullptr; }

    VkImage GetVkImage() const { return Image; }
    VmaAllocation GetAllocation() const { return Allocation; }

private:
    VkImage Image = nullptr;
    VmaAllocation Allocation = nullptr;
};

class VulkanImageView final : public RHIImageView
{
public:
    explicit VulkanImageView(VkImageView ImageViewIn, const RHIImageViewDesc& DescIn)
        : RHIImageView(DescIn)
          , ImageView(ImageViewIn)
    {
    }

    bool IsValid() const override { return ImageView != nullptr; }
    VkImageView GetVkImageView() const { return ImageView; }

private:
    VkImageView ImageView = nullptr;
};

class VulkanWindow final : public RHIWindow
{
public:
    VulkanWindow(RefCountingPtr<RHIImage> InDepthImage, RefCountingPtr<RHIImageView> InDepthImageView,
              std::vector<RefCountingPtr<RHIImage>> InSwapchainImages, std::vector<RefCountingPtr<RHIImageView>> InSwapchainImageViews,
              RHIFormat InFormat, uint32 InWidth, uint32 InHeight, VkSurfaceKHR InWindowsSurface, VkSwapchainKHR InSwapchain)
        : RHIWindow(InDepthImage, InDepthImageView, InSwapchainImages, InSwapchainImageViews, InFormat, InWidth, InHeight),
          WindowSurface(InWindowsSurface),
          Swapchain(InSwapchain)
    {
    }

    bool IsValid() const override { return WindowSurface != nullptr && Swapchain != nullptr; }

    VkSurfaceKHR GetVkSurface() const { return WindowSurface; }
    VkSwapchainKHR GetVkSwapchain() const { return Swapchain; }
    VkSwapchainKHR* GetVkSwapchainPtr() { return &Swapchain; }

private:
    VkSurfaceKHR WindowSurface = nullptr;
    VkSwapchainKHR Swapchain = nullptr;
};

#define DECLARE_VULKAN_RESOURCE_CASTER(VulkanTypeImplementation, RHIType) \
	template <> \
	struct VulkanResourceCaster<RHIType> \
	{ \
		using VulkanType = VulkanTypeImplementation; \
	};

template <class T>
struct VulkanResourceCaster
{
};

DECLARE_VULKAN_RESOURCE_CASTER(VulkanGlobalBuffer, RHIGlobalBuffer)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanLinearBuffer, RHILinearBuffer)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanSubAllocation, RHIBufferSubAllocation)

DECLARE_VULKAN_RESOURCE_CASTER(VmaVirtualMemoryBlock, RHIVirtualMemoryBlock)

DECLARE_VULKAN_RESOURCE_CASTER(VmaVirtualMemoryAllocation, RHIVirtualMemoryAllocation)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanCommandList, RHICommandList)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanGlobalBufferChannel, RHIGlobalBufferChannel)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanPipelineLayout, RHIPipelineLayout)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanPipelineObject, RHIPipelineObject)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanImage, RHIImage)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanImageView, RHIImageView)

DECLARE_VULKAN_RESOURCE_CASTER(VulkanWindow, RHIWindow)

template <typename RHIType>
static typename VulkanResourceCaster<RHIType>::VulkanType* ResourceCast(RHIType* Resource)
{
    return static_cast<typename VulkanResourceCaster<RHIType>::VulkanType*>(Resource);
}
}
